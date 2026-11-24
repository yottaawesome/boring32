// https://docs.microsoft.com/en-us/windows/win32/wic/-wic-lh
// Make sure to link against Windowscodecs.lib
// See https://docs.microsoft.com/en-us/windows/win32/wic/-wic-api
export module Boring32:WindowsImagingComponent;
export import :WindowsImagingComponent.ImagingFactory;
