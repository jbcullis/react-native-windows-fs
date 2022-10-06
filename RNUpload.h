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
    // Represent the options object passed to upload()
    struct RNUploadOptions
    {
        std::optional<std::string> maxWidth = std::nullopt;
        std::optional<std::string> maxHeight = std::nullopt;
    };

    REACT_MODULE(RNUpload);
    struct RNUpload
    {
        inline static constexpr std::string_view Name = "RNUpload";

        ReactContext reactContext = nullptr;

        REACT_INIT(RNUpload_Init);
        void RNUpload_Init(ReactContext const& context) noexcept
        {
            reactContext = context;
        }

        // Asynchronously saves the file.
        IAsyncAction UploadAsyncHelper(
            RNUploadOptions options,
            ReactPromise<JSValueObject> promise) noexcept;

        REACT_METHOD(Upload, L"Upload");
        void Upload(JSValueObject&& options, ReactPromise<JSValueObject> promise) noexcept;

    };
}