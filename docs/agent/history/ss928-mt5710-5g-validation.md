### skills/ss928-mt5710-5g-validation

- 目标：把 SS928 上 TD Tech `MT5710/TDTECH_MT571X` 5G RedCap 模组的板端验证流程固化成可复用 skill。
- 关键文件：`skills/ss928-mt5710-5g-validation/SKILL.md`；元数据在 `skills/ss928-mt5710-5g-validation/agents/openai.yaml`。
- 已验证可用：装好外置天线后，移动 SIM 卡使用 `cmnet` 拨号成功，NCM 网口 DHCP 获取 `10.72.12.12`，运营商 DNS 为 `223.87.253.100`/`223.87.253.253`，`223.5.5.5` 和 `www.baidu.com` 均 5/5 包通过，说明流量上网和互联网连通已通。
- 已验证可用：`/dev/ttyUSB1` PCUI AT 通道正常，`ATD18180871724;` 语音呼叫信令可从外呼/振铃进入 active，`ATH` 可挂断；音频通路未接麦克风/喇叭或 PCM，因此只确认信令，不确认声音。
- 已验证不可用：当前 `MT5710_CN V100R001C00B095` Mini-PCIe 固件对 `AT^GNSSENABLE`、`AT^GNSSGETINFO`、`AT^GNSSGETNMEA` 以及常见 `CGPS/CGNS/QGPS` 命令均返回不支持，`/dev/ttyUSB3` 也无 NMEA 输出；定位/GNSS 目前按固件或产品变体限制处理。
- 踩坑：不要固定写死 `usb0`，本次实际 NCM 网口为 `enx7ecca5347f7b`；无天线时 `^HCSQ` 曾只有约 `-126 dBm` 且无法稳定注册，装天线后约 `-91 dBm` 并成功驻网拨号；不要把 RM500U/Quectel 命令直接套到 MT5710。
- SIM/APN 规则：当前实测是移动卡 `cmnet`；后续换电信普通数据卡优先用 `ctnet`，换联通普通数据卡优先用 `3gnet`，物联网卡或专网卡必须以运营商下发 APN 为准；APN 属于 SIM/套餐属性，不属于 MT5710 模组固定属性。
