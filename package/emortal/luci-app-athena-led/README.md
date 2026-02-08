# luci-app-athena-led
jdCloud ax6600 led screen ctrl

#### 定时关闭开启代码

```
# 早上9点30开启显示
30 9     * *  *   uci set athena_led.config.enable='1' && uci commit athena_led &&  /etc/init.d/athena_led reload
# 下午6点30关闭显示
30 18   * *  *  uci set athena_led.config.enable='0' && uci commit athena_led &&  /etc/init.d/athena_led reload
```

[推荐固件下载地址](https://github.com/VIKINGYFY/OpenWRT-CI/releases)


![image](https://github.com/user-attachments/assets/a2bcf6af-4e29-49d4-b183-799f68b74efb)



感谢以下朋友的捐赠
1. *姆
