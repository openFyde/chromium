`app.html` 是入口，进行了必要的修改，以合并自己开发的 js 和 css 文件，其中最大的不同是额外添加了一个 `app_bin.css` 的引用。

`app.html` 中引入的 js 和 css 文件路径名由 `ash/webui/help_app_ui/help_app_untrusted_ui.cc` 中指定。

```cpp
# 第一个参数是 app.html 引用时的文件路径名
source->AddResourcePath("app_bin.js", IDR_HELP_APP_APP_BIN_JS);
source->AddResourcePath("app_bin.css", IDR_HELP_APP_APP_BIN_CSS);
source->AddResourcePath("load_time_data.js", IDR_WEBUI_JS_LOAD_TIME_DATA_JS);
source->AddResourcePath("help_app_app_scripts.js",
```

上面代码中的 `IDR_HELP_APP_APP_BIN_JS` 等符号，由 `ash/webui/help_app_ui/resources/mock/help_app_bundle_mock_resources.grd` 中指定。

```xml
<include name="IDR_HELP_APP_APP_BIN_JS" file="fydeos_app_bin.js" type="BINDATA" />
<include name="IDR_HELP_APP_APP_BIN_CSS" file="fydeos_app_bin.css" type="BINDATA" />
```

上面代码中的 `fydeos_app_bin.js` 等文件名可以为任意文件名，不必跟上述 `app.html` 中引用的文件同名。对应的文件放在跟 `help_app_bundle_mock_resources.grd` 相同的目录，即 `chromeos/components/help_app_ui/resources/mock/`。


由普通开发流程构建得来的资源文件，就放置在此处，并且在 `help_app_bundle_mock_resources.grd` 引入。

如果之后不需要在引入新的资源文件，那么只需要更新 `fydeos_app_bin.js` 和 `fydeos_app_bin.css` 文件即可。


---

（如果要引入新的资源文件，需要修改 `help_app_bundle_mock_resources.grd`，并且在 `help_app_untrusted_ui.cc` 添加。 然后可能需要在 `app.html` 引用新的文件，如果新的文件由 `fydeos_app_main.js` 引入，那么不必修改其他文件，但是要注意在引用资源文件时，其路径由 `help_app_untrusted_ui.cc` 的代码确定。）
