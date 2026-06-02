```markdown
## 问题背景

OpenWrt 编译框架在下载 `alsa-utils-1.2.15.2.tar.bz2` 时，`download.pl`
调用 curl 连接 `alsa-project.org` 触发 TLS 握手失败：

```
curl: (35) TLS connect error: error:0A000410:SSL routines::ssl/tls alert handshake failure
```

**根因**：curl 8.15.0 + OpenSSL 3.5.4 默认发起 TLS 1.3 握手，
但 alsa-project.org 服务器的 TLS 1.3 配置存在问题，导致握手被拒绝。
手动加 `--tlsv1.2` 可正常连接，说明服务器侧 TLS 1.2 工作正常。

---

## 方案对比

| 方案 | 侵入性 | 持久性 | 风险 |
|---|---|---|---|
| 修改 `download.pl` | 高，改核心脚本 | 会被 `git pull` 覆盖 | 影响所有下载逻辑 |
| `~/.config/curl/curlrc` | **零**，用户级配置 | 永久，git 完全感知不到 | 仅锁定当前用户 curl 行为 |
| 修改系统 OpenSSL 策略 | 中，改系统配置 | 永久 | 影响全系统所有 TLS 连接 |

---

## 修复方案：curl 用户级配置

编译框架调用的所有 curl 均会自动读取 `~/.config/curl/curlrc`，
无需修改任何项目文件或脚本。

```bash
mkdir -p ~/.config/curl
echo "tlsv1.2" >> ~/.config/curl/curlrc
```

验证生效：

```bash
curl -v https://www.alsa-project.org/ 2>&1 | grep 'SSL connection using'
# 期望输出：* SSL connection using TLSv1.2 / ...
```

---

## 注意事项

- 此配置锁定该用户所有 curl 调用默认使用 TLS 1.2
- 若未来某服务器**仅支持 TLS 1.3**，需临时用 `--tlsv1.3` 覆盖
- 该文件不在任何 git 仓库追踪范围内，升级 OpenWrt 源码不受影响
```
