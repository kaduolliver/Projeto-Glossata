param([string]$PackageDirectory = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (!$PackageDirectory) { $PackageDirectory = Join-Path $projectRoot 'package' }
$packagePath = (Resolve-Path -LiteralPath $PackageDirectory).Path
$executable = Join-Path $packagePath 'projeto_glossata.exe'
if (!(Test-Path -LiteralPath $executable)) { throw 'Executável empacotado não encontrado.' }
$reportPath = Join-Path $projectRoot 'build/package-smoke.json'
$stdoutPath = Join-Path $projectRoot 'build/package-smoke.stdout.log'
$stderrPath = Join-Path $projectRoot 'build/package-smoke.stderr.log'
$savedPath = $env:PATH
$savedQtVariables = @{}
Get-ChildItem Env: | Where-Object { $_.Name -match '^(QT_|QML|QTDIR)' } | ForEach-Object {
    $savedQtVariables[$_.Name] = $_.Value
}
try {
    foreach ($name in $savedQtVariables.Keys) { Remove-Item -LiteralPath "Env:$name" }
    $env:PATH = "$packagePath;$env:SystemRoot\System32;$env:SystemRoot"
    $env:QT_QPA_PLATFORM = 'offscreen'
    $env:QT_QUICK_BACKEND = 'software'
    $env:QT_FORCE_STDERR_LOGGING = '1'
    $testProcess = Start-Process -FilePath $executable -ArgumentList '--smoke-test' -WorkingDirectory $packagePath -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
    $null = $testProcess.Handle
    Start-Sleep -Milliseconds 300
    $modulePaths = @()
    if (!$testProcess.HasExited) { $modulePaths = @($testProcess.Modules | ForEach-Object { $_.FileName }) }
    if (!$testProcess.WaitForExit(45000)) {
        # Only the process created above is terminated; a user's game is untouched.
        $testProcess.Kill()
        throw 'O teste isolado excedeu 45 segundos.'
    }
    $testProcess.Refresh()
    $report = [ordered]@{
        executable = $executable
        exit_code = $testProcess.ExitCode
        environment = 'Package + Windows system directories in PATH; offscreen/software'
        scope = 'Startup, bundled QML/catalog, SQLite, competitive question availability. Not a clean Windows installation test.'
        loaded_modules = $modulePaths
        stderr = [System.IO.File]::ReadAllText($stderrPath)
    }
    $report | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $reportPath -Encoding UTF8
    if ($testProcess.ExitCode -ne 0) { throw "Falha ao executar o pacote: $($testProcess.ExitCode). Consulte $stderrPath" }
    if ($report.stderr -match 'ReferenceError|TypeError|failed to load|module .* not installed|Cannot load library') { throw "Aviso de carregamento: consulte $stderrPath" }
    if (@($modulePaths | Where-Object { $_ -match '\\msys64\\' }).Count) { throw 'O executável carregou dependências do MSYS2 externo.' }
    Write-Host 'Pacote iniciado com PATH isolado; relatório em build/package-smoke.json.'
}
finally {
    Get-ChildItem Env: | Where-Object { $_.Name -match '^(QT_|QML|QTDIR)' } | ForEach-Object { Remove-Item -LiteralPath "Env:$($_.Name)" }
    foreach ($name in $savedQtVariables.Keys) { [Environment]::SetEnvironmentVariable($name,$savedQtVariables[$name],'Process') }
    $env:PATH = $savedPath
}
