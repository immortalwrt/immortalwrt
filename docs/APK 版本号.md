```markdown
# base-files APK 版本号修复记录

## 问题描述

`apk mkpkg` 打包时报错，版本号包含非法字符：

- `VERSION` 由 `COMMITCOUNT` + `REVISION` 拼接而成
- `REVISION` 格式如 `r12345-abcdef`，含 `-` 分隔符
- 最终版本号形如 `12345.abcdef` 或带 `~` 的预发布标记
- APK 规范要求版本号为纯数字或符合 Alpine 格式，上述格式均不合法

---

## APK 版本号规范细则

APK（Alpine Package Keeper）版本号遵循以下规则：

### 合法格式

```
<digit>{.<digit>}[<letter>][_<suffix>[<digit>]][~<pre>][<digit>][-r<digit>]
```

### 各字段说明

| 字段 | 格式 | 示例 | 说明 |
|------|------|------|------|
| 主版本 | 数字，可多段用 `.` 分隔 | `1.2.3` | 必须以数字开头 |
| 字母后缀 | 单个小写字母 | `1.2a` | 紧跟最后一段数字，可选 |
| 预发布标记 | `_alpha` `_beta` `_pre` `_rc` `_p` | `1.2_rc1` | 下划线开头，可选 |
| 波浪号预发布 | `~<字符串>` | `1.2~dev` | 版本排序低于正式版，**APK v3 才支持** |
| Revision | `-r<数字>` | `1.2-r3` | 包的修订号，非上游版本号的一部分 |

### 非法字符

| 字符 | 原因 |
|------|------|
| `-` （非 `-r` 形式）| 连字符仅允许出现在 `-r<digit>` revision 段 |
| `~` | 部分 APK 版本不支持波浪号预发布格式 |
| 字母开头 | 版本号必须以数字开头，如 `abcdef` 不合法 |
| 大写字母 | 字母后缀只允许单个小写字母 |

### 版本排序规则

```
~pre  <  正式版  <  _alpha  <  _beta  <  _pre  <  _rc  <  (无后缀)  <  _p
```

示例排序：
```
1.0~dev < 1.0_alpha1 < 1.0_beta1 < 1.0_rc1 < 1.0 < 1.0_p1 < 1.0a < 1.1
```

---

## 修复方案

将 `VERSION` 简化为纯数字 `$(COMMITCOUNT)`，同步修正 `STAGING_DIR` 中的版本写入逻辑。

## 修复命令

```bash
# 修改 VERSION 定义（第50行）
sed -i 's|VERSION:=$(COMMITCOUNT).$(lastword $(subst -, ,$(REVISION)))|VERSION:=$(COMMITCOUNT)|' \
    ~/OpenWrt/zagwrt/package/base-files/Makefile

# 修改 STAGING_DIR 版本写入逻辑（第260行）
sed -i '260s/.*/\techo $(COMMITCOUNT) >$(STAGING_DIR)\/base-files.version/' \
    ~/OpenWrt/zagwrt/package/base-files/Makefile

# 清理并重新编译
rm -rf ~/OpenWrt/zagwrt/build_dir/target-x86_64_musl/linux-x86_64/base-files/
make package/base-files/compile V=s
```

## 验证

```bash
grep "VERSION" ~/OpenWrt/zagwrt/package/base-files/Makefile | head -5
```

预期第50行输出：

```
VERSION:=$(COMMITCOUNT)
```

---

## 注意事项

- 此修改仅影响 APK 包版本号显示，不影响固件功能
- `COMMITCOUNT` 为纯数字（git commit 计数），满足 APK 版本号规范
- 若后续需要更丰富的版本信息，可考虑使用 `PKG_RELEASE` 字段补充
- APK v2（OpenWrt 当前使用）不支持 `~` 波浪号格式，需严格避免
```
