param([string]$QtBin = 'C:\msys64\ucrt64\bin', [switch]$Development,
      [string]$BuildBinary = '', [string]$PackageDirectory = '')

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (!$Development) {
    & python (Join-Path $PSScriptRoot 'distribution.py') check
    if ($LASTEXITCODE -ne 0) { throw 'Distribuição bloqueada por pendências de evidência. -Development prepara somente um pacote local de teste.' }
}
$packagePath = if ($PackageDirectory) { [System.IO.Path]::GetFullPath($PackageDirectory) } else { Join-Path $projectRoot 'package' }
if ($PackageDirectory) {
    $workspacePrefix = [System.IO.Path]::GetFullPath($projectRoot).TrimEnd('\') + '\'
    if (!$packagePath.StartsWith($workspacePrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw 'O diretório alternativo de validação deve permanecer dentro do projeto.'
    }
    if (Test-Path -LiteralPath $packagePath) {
        throw 'Use um diretório alternativo novo para preservar pacotes e artefatos existentes.'
    }
}
$executable = Join-Path $packagePath 'projeto_glossata.exe'
$deployTool = Join-Path $QtBin 'windeployqt6.exe'
$objdump = Join-Path $QtBin 'objdump.exe'
New-Item -ItemType Directory -Path $packagePath -Force | Out-Null
Set-Content -LiteralPath (Join-Path $packagePath 'LOCAL_DEVELOPMENT_ONLY.txt') -Encoding UTF8 -Value 'Pacote em preparação ou uso local de desenvolvimento. Não redistribuir sem concluir o gate de permissões e obrigações.'
$sourceExecutable = if ($BuildBinary) { [System.IO.Path]::GetFullPath($BuildBinary) } else { Join-Path $projectRoot 'build/projeto_glossata.exe' }
Copy-Item -LiteralPath $sourceExecutable -Destination $executable -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'resources/data/discord.json') -Destination (Join-Path $packagePath 'discord.json') -Force
$shortcutShell = New-Object -ComObject WScript.Shell
$iconSource = Join-Path $projectRoot 'resources/branding/glossata.ico'
$iconHash = (Get-FileHash -LiteralPath $iconSource -Algorithm SHA256).Hash.Substring(0, 12).ToLowerInvariant()
$packageBrandingPath = Join-Path $packagePath 'branding'
New-Item -ItemType Directory -Path $packageBrandingPath -Force | Out-Null
$launcherIcon = Join-Path $packageBrandingPath "glossata-$iconHash.ico"
Copy-Item -LiteralPath $iconSource -Destination $launcherIcon -Force
$shortcut = $shortcutShell.CreateShortcut((Join-Path $packagePath 'Projeto Glossata.lnk'))
$shortcut.TargetPath = $executable
$shortcut.WorkingDirectory = $packagePath
$shortcut.IconLocation = "$launcherIcon,0"
$shortcut.Description = 'Iniciar Projeto Glossata'
$shortcut.Save()
if (-not ('GlossataShellNotify' -as [type])) {
    Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; public static class GlossataShellNotify { [DllImport("shell32.dll", CharSet=CharSet.Unicode)] public static extern void SHChangeNotify(uint eventId, uint flags, string path, IntPtr otherItem); }'
}
# Refresh only this shortcut. A content-based icon filename avoids stale EXE icons.
[GlossataShellNotify]::SHChangeNotify(0x00002000, 0x00001005, (Join-Path $packagePath 'Projeto Glossata.lnk'), [IntPtr]::Zero)
[GlossataShellNotify]::SHChangeNotify(0x00002000, 0x00001005, $executable, [IntPtr]::Zero)
# Reload executable icons as well as the versioned shortcut icon.
[GlossataShellNotify]::SHChangeNotify(0x08000000, 0, $null, [IntPtr]::Zero)
& $deployTool --release --verbose 0 --include-plugins qoffscreen --exclude-plugins qsqlibase,qsqlmysql,qsqlodbc,qsqlpsql --qmldir (Join-Path $projectRoot 'resources/qml') $executable
if ($LASTEXITCODE -ne 0) { throw 'Falha no windeployqt6.' }
if (!(Test-Path -LiteralPath (Join-Path $packagePath 'Qt6Multimedia.dll'))) {
    throw 'Qt Multimedia ausente no pacote.'
}
# MSYS2's compiled import prefix points at its installation. Keep the package
# relocatable and let the isolated smoke test resolve only deployed QML/plugins.
Set-Content -LiteralPath (Join-Path $packagePath 'qt.conf') -Encoding Ascii -Value @('[Paths]', 'Prefix=.', 'Plugins=.', 'QmlImports=qml')

# Remove unused SQL plugins left by earlier deployments; the game uses QSQLITE.
foreach ($unusedDriver in 'qsqlibase.dll', 'qsqlmysql.dll', 'qsqlodbc.dll', 'qsqlpsql.dll') {
    $unusedDriverPath = Join-Path $packagePath "sqldrivers/$unusedDriver"
    if (Test-Path -LiteralPath $unusedDriverPath) {
        Remove-Item -LiteralPath $unusedDriverPath
    }
}

# MSYS2 Qt also depends on non-Qt libraries that windeployqt may omit.
$pendingBinaries = [System.Collections.Generic.Queue[string]]::new()
$visitedBinaries = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
Get-ChildItem -LiteralPath $packagePath -Recurse -File |
    Where-Object { $_.Extension -in '.dll', '.exe' } |
    ForEach-Object { $pendingBinaries.Enqueue($_.FullName) }
while ($pendingBinaries.Count -gt 0) {
    $binaryPath = $pendingBinaries.Dequeue()
    if (!$visitedBinaries.Add($binaryPath)) { continue }
    $imports = & $objdump -p $binaryPath
    if ($LASTEXITCODE -ne 0) { throw "Falha ao inspecionar $binaryPath" }
    foreach ($importLine in $imports) {
        if ($importLine -match 'DLL Name:\s*(\S+)') {
            $libraryName = $Matches[1]
            $sourceLibrary = Join-Path $QtBin $libraryName
            $targetLibrary = Join-Path $packagePath $libraryName
            if ((Test-Path -LiteralPath $sourceLibrary) -and !$libraryName.StartsWith('Qt6')) {
                if (!$visitedBinaries.Contains($targetLibrary)) {
                    Copy-Item -LiteralPath $sourceLibrary -Destination $targetLibrary -Force
                    $pendingBinaries.Enqueue($targetLibrary)
                }
            }
        }
    }
}
# Remove only generated launchers from the previous application name.
foreach ($legacyName in 'Glossata Quiz.lnk', 'glossata_quiz.exe') {
    $legacyPath = Join-Path $packagePath $legacyName
    if (Test-Path -LiteralPath $legacyPath) {
        Remove-Item -LiteralPath $legacyPath
    }
}
Write-Host "Pacote atualizado em $packagePath"
& python (Join-Path $PSScriptRoot 'distribution.py') export-local --package $packagePath --prefix (Split-Path -Parent $QtBin)
if ($LASTEXITCODE -ne 0) { throw 'Falha ao exportar créditos, evidências ou inventário de dependências.' }
if (!$Development) {
    & python (Join-Path $PSScriptRoot 'distribution.py') check --package $packagePath
    if ($LASTEXITCODE -ne 0) { throw 'Distribuição bloqueada após inventário do pacote.' }
    Remove-Item -LiteralPath (Join-Path $packagePath 'LOCAL_DEVELOPMENT_ONLY.txt')
}
