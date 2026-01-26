$repoRoot = Resolve-Path "E:\Code\C++\boring32"
Set-Location $repoRoot

$targetDir = "src\Boring32\Async"
# Find files matching the pattern boring32-async_*
# This includes .ixx and potentially .cpp files if they exist
$files = Get-ChildItem -Path $targetDir -Filter "boring32-async_*"

foreach ($file in $files) {
    # Replace the underscore separator with a period
    if ($file.Name -match "^boring32-async_") {
        $newName = $file.Name -replace "^boring32-async_", "boring32-async."
        Write-Host "Renaming $($file.Name) to $newName"
        git mv "$targetDir\$($file.Name)" "$targetDir\$newName"
    }
}

# Update Visual Studio project files to reference the new filenames
$projectFiles = @("Boring32\Boring32.vcxproj", "Boring32\Boring32.vcxproj.filters")
foreach ($path in $projectFiles) {
    $fullPath = Join-Path $repoRoot $path
    if (Test-Path $fullPath) {
        Write-Host "Updating project file: $path"
        $content = Get-Content $fullPath -Raw
        # Replace the filename prefix in the project files
        $newContent = $content.Replace("boring32-async_", "boring32-async.")
        Set-Content -Path $fullPath -Value $newContent -Encoding UTF8
    }
}

Write-Host "Renaming complete. Please verify the changes and reload the project in Visual Studio."