# HLK-LD2417 雷达 Web 上位机

这是一个运行在电脑浏览器里的 HLK-LD2417 上位机原型，使用 Web Serial API 读取串口数据，并把目标画到扇形雷达图里。

## 运行

在本目录启动本地服务，然后用 Chrome 或 Edge 打开：

```powershell
python .\serve.py
```

访问：

```text
http://127.0.0.1:8765/
```

Web Serial API 需要安全上下文，`localhost` / `127.0.0.1` 可以直接使用。

## 串口参数

- 电平：TTL 串口，需要 USB-TTL 转接板
- 默认波特率：115200
- 数据位：8
- 停止位：1
- 校验：无
- 字节序：小端

## 已实现协议

上报帧按《HLK-LD2417 串口协议说明 V1.0.0》解析：

```text
AA AA | N | target[0..N-1] | 55 55
```

单个目标 8 字节：

```text
目标编号: 1 byte
目标来向: 1 byte, 1=左, 2=右
目标 y 坐标/距离: uint16 little-endian, m * 100
目标速度: uint16 little-endian, km/h * 100
目标状态: uint16 little-endian
```

状态位：

- bit0: 是否高速
- bit1: 当前帧是否关联成功
- bit8-15: 轨迹持续时间/帧计数

## 角度说明

使用手册给出的探测角度是水平 ±60°、俯仰 ±20°。当前串口上报协议没有给出连续角度字段，也没有给出 x 坐标，只给出左/右来向。因此页面按 ±60° 绘制探测扇区，并把左/右来向映射到可调的显示角度：

- 左来向默认：-30°
- 右来向默认：30°

如果后续拿到包含真实角度或 x/y 坐标的新协议，只需要扩展 `parser.js` 的目标字段，再在 `app.js` 的 `displayAngleFor()` 中使用真实角度即可。

## 文件

- `index.html`: 页面结构
- `styles.css`: 页面样式
- `parser.js`: LD2417 二进制帧解析
- `app.js`: 串口读取、雷达绘制、表格和演示数据
- `test_parser.js`: 解析器自检
- `serve.py`: 本地静态服务
