import json
import scipy.io
import os

# === 1. 加载并诊断JSON文件 ===
json_path = '../src/datasets/ImageNet/imagenet_class_index.json'
with open(json_path, 'r') as f:
    data = json.load(f)

print(f"JSON文件类型: {type(data)}")
print(f"JSON内容预览（第一个元素）: {list(data.items())[0] if isinstance(data, dict) else data[0]}")

# === 2. 适配不同格式，构建 wnid -> index 映射 ===
wnid_to_pytorch_idx = {}
if isinstance(data, dict):
    # 如果文件是字典: {"0": ["n01440764", "tench"], "1": [...], ...}
    for idx_str, (wnid, _) in data.items():
        wnid_to_pytorch_idx[wnid] = int(idx_str)
elif isinstance(data, list):
    # 如果文件是列表: [["n01440764", "tench"], [...], ...]
    for idx, (wnid, _) in enumerate(data):
        wnid_to_pytorch_idx[wnid] = idx
else:
    raise ValueError("JSON格式无法识别，既不是字典也不是列表。")

print(f"成功构建 PyTorch 索引映射，共 {len(wnid_to_pytorch_idx)} 个类别。")

# === 3. 加载并过滤 meta.mat (仅ILSVRC2012的1000类) ===
devkit_path = '../src/datasets/ImageNet/ILSVRC2012_devkit_t12'  # 请修改为你的路径
meta = scipy.io.loadmat(os.path.join(devkit_path, 'data', 'meta.mat'), squeeze_me=True)['synsets']
id_to_wnid = {}
for item in meta:
    ilsvrc_id = item[0]
    if 1 <= ilsvrc_id <= 1000:  # 关键过滤
        id_to_wnid[ilsvrc_id] = item[1]
print(f"过滤得到 ILSVRC2012 类别: {len(id_to_wnid)} 个。")

# === 4. 转换原始标签 ===
gt_file = os.path.join(devkit_path, 'data', 'ILSVRC2012_validation_ground_truth.txt')
with open(gt_file, 'r') as f:
    original_labels = [int(line.strip()) for line in f]

output_lines = []
missing = 0
for i, original_id in enumerate(original_labels):
    wnid = id_to_wnid.get(original_id)
    if wnid is None:
        missing += 1
        continue
    pytorch_idx = wnid_to_pytorch_idx.get(wnid)
    if pytorch_idx is None:
        missing += 1
        continue
    filename = f"ILSVRC2012_val_{i+1:08d}.JPEG"
    output_lines.append(f"{filename} {pytorch_idx}\n")

# === 5. 保存结果 ===
with open('../src/datasets/ImageNet/val_label.txt', 'w') as f:
    f.writelines(output_lines)

print(f"\n转换完成！")
print(f"处理图片: {len(original_labels)}， 成功转换: {len(output_lines)}， 失败: {missing}")
if missing == 0:
    print("✅ 全部转换成功！")
    print("生成文件前5行预览:")
    for line in output_lines[:5]:
        print(f"  {line.strip()}")