## References:
1. [GN quick start](https://chromium.googlesource.com/chromium/src/+/master/tools/gn/docs/quick_start.md)
2. [GN reference](https://chromium.googlesource.com/chromium/src/+/master/tools/gn/docs/reference.md)
3. [add a new API to Chrome](https://cs.chromium.org/chromium/src/chrome/browser/extensions/api/README.txt?type=cs)

## Detail:
1. Root gn file `src/chrome/common/extensions/api/BUILD.gn`
2. the c++ files in browser/api is compiled by `src/fydeos/extensions/browser/BUILD.gn` and add url to `src/chrome/browser/extensions/BUILD.gn`
3. Add more function objects to extension:
* Add const names of new functions to `src/extensions/browser/extension_function_histogram_value.h`
* running script `python tools/metrics/histograms/update_extension_histograms.py`
4. Add new permissions to extension:
* Add extension permission's ID to `src/extensions/common/permissions/api_permission.h`
* running script `python tools/metrics/histograms/update_extension_permission.py`
* Add register command to `APIPermissionInfo::InitInfo permissions_to_register` in `src/extensions/common/extensions_api_permissions.cc`
5. Add check permission command at the front of real private functions.
6. Add permission information to `chrome/app/fydeos_generated_resources.grdp` append ids to  `chrome/common/extensions/permissions/chrome_permission_message_rules.cc` 


## About BUILD.gn
1. you can choise any name for lib, but you have to write one group named as same as the parent directory, add your lib or some object to its deps.

2. hashed id for white list:

```
echo -n "APPID(32bit md5 strings)" |shasum - |tr '[:lower:]' '[:upper:]'
```

## Add event to extension:
1. Add const value of Event to `extensions/browser/extension_event_histogram_value.h`
2. run `python tools/metrics/histograms/update_extension_histograms.py` to update related xml file.
3. regist your event router as service at `extensions/browser/browser_context_keyed_service_factories.cc`
