# react-native-windows-fs


Usage:

**Get base64 from file**
let _Response = await NativeModules.RNUpload.Upload(1200, 1200);

**Export base64 to file**
await NativeModules.RNExport.Export(fileName, base64);
