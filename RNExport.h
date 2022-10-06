#pragma once

#include "pch.h"
#include "NativeModules.h"

#include <functional>
#include <sstream>
#include <mutex>
#include <string_view>

#include "JSValue.h"

using namespace std;
using namespace winrt::Microsoft::ReactNative;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

namespace winrt::livestocked::implementation
{
    // Represent the options object passed to export()
    struct RNExportOptions
    {
        std::optional<std::string> FileName = std::nullopt;
        std::optional<std::string> FileExtension = std::nullopt;
        std::optional<std::string> Base64 = std::nullopt;
    };

    REACT_MODULE(RNExport);
    struct RNExport
    {
        inline static constexpr std::string_view Name = "RNExport";

        ReactContext reactContext = nullptr;

        REACT_INIT(RNUpload_Init);
        void RNUpload_Init(ReactContext const& context) noexcept
        {
            reactContext = context;
        }

        // Asynchronously saves the file.
        IAsyncAction ExportAsyncHelper(
            RNExportOptions options,
            ReactPromise<JSValue> promise) noexcept;

        REACT_METHOD(Export, L"Export");
        void Export(JSValueObject&& options, ReactPromise<JSValue> promise) noexcept;

    };
}