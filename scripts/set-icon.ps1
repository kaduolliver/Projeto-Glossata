param([Parameter(Mandatory = $true)][string]$InputImage)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$projectRoot = Split-Path -Parent $PSScriptRoot
$brandingPath = Join-Path $projectRoot 'resources/branding'
New-Item -ItemType Directory -Path $brandingPath -Force | Out-Null
$sourceImage = [System.Drawing.Image]::FromFile((Resolve-Path -LiteralPath $InputImage).Path)
$frames = [System.Collections.Generic.List[byte[]]]::new()
$sizes = @(16, 24, 32, 48, 64, 128, 256)
try {
    foreach ($size in @($sizes) + @(1024)) {
        $bitmap = [System.Drawing.Bitmap]::new($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
        try {
            $graphics.Clear([System.Drawing.Color]::Transparent)
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            $scale = [Math]::Min($size / $sourceImage.Width, $size / $sourceImage.Height)
            $scaledWidth = [float]($sourceImage.Width * $scale)
            $scaledHeight = [float]($sourceImage.Height * $scale)
            $graphics.DrawImage($sourceImage, [float](($size - $scaledWidth) / 2), [float](($size - $scaledHeight) / 2), $scaledWidth, $scaledHeight)
            if ($size -eq 1024) {
                $bitmap.Save((Join-Path $brandingPath 'glossata.png'), [System.Drawing.Imaging.ImageFormat]::Png)
            } else {
                $memory = [System.IO.MemoryStream]::new()
                try {
                    $bitmap.Save($memory, [System.Drawing.Imaging.ImageFormat]::Png)
                    $frames.Add($memory.ToArray())
                } finally { $memory.Dispose() }
            }
        } finally { $graphics.Dispose(); $bitmap.Dispose() }
    }
} finally { $sourceImage.Dispose() }

$iconPath = Join-Path $brandingPath 'glossata.ico'
$iconStream = [System.IO.File]::Create($iconPath)
$writer = [System.IO.BinaryWriter]::new($iconStream)
try {
    $writer.Write([uint16]0)
    $writer.Write([uint16]1)
    $writer.Write([uint16]$sizes.Count)
    $offset = [uint32](6 + 16 * $sizes.Count)
    for ($i = 0; $i -lt $sizes.Count; ++$i) {
        $sizeByte = if ($sizes[$i] -eq 256) { [byte]0 } else { [byte]$sizes[$i] }
        $writer.Write($sizeByte); $writer.Write($sizeByte)
        $writer.Write([byte]0); $writer.Write([byte]0)
        $writer.Write([uint16]1); $writer.Write([uint16]32)
        $writer.Write([uint32]$frames[$i].Length); $writer.Write($offset)
        $offset += [uint32]$frames[$i].Length
    }
    foreach ($frame in $frames) { $writer.Write([byte[]]$frame) }
} finally { $writer.Dispose(); $iconStream.Dispose() }
Write-Host "Ícone Windows: $iconPath"
Write-Host "Imagem Discord (1024 x 1024): $(Join-Path $brandingPath 'glossata.png')"
