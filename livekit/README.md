# LiveKit Server 与 Gemini Voice AI Agent 本地部署指南

本目录包含 LiveKit 媒体服务器与基于 **Google Gemini Live (Speech-to-Speech)** 的全双工实时语音 AI Agent。

---

## 1. 目录结构

```
livekit/
├── README.md           # 本指南
├── livekit_ai_agent.md # LiveKit AI Agent 实时语音数据流流转架构拆解
├── livekit.yaml        # LiveKit Server 本地开发配置文件
├── agent.py            # 基于 Google Gemini Live API 的全双工语音 Agent
├── requirements.txt    # Python Agent 依赖
└── .env.example        # 环境变量模板
```

---

## 2. 核心技术架构 (Gemini Live API)

本项目基于 **Speech-to-Speech (S2S) 端到端全双工架构**：
- **模型**：Google `gemini-2.5-flash-native-audio-preview-12-2025` / `gemini-3.8-live`
- **媒体流传输**：基于 WebRTC 原生 Opus 48kHz 单声道音频轨直传 LiveKit SFU
- **打断处理 (Barge-in)**：本地加载 Silero VAD 毫秒级语音活性检测，实现用户插话即时截断
- **端到端延时**：约 300ms ~ 600ms，无需拼接 STT 与 TTS

---

## 3. 准备工作

### 3.1 获取 Google API Key
访问 [Google AI Studio](https://aistudio.google.com/apikey) 生成免费的 Gemini API Key。

### 3.2 配置环境变量
在 `livekit` 目录下创建 `.env.local` 文件（或 `.env`）：

```bash
cp .env.example .env.local
```

打开 `.env.local` 并填入您的 `GOOGLE_API_KEY`：

```bash
LIVEKIT_URL=ws://127.0.0.1:7880
LIVEKIT_API_KEY=devkey
LIVEKIT_API_SECRET=secret_livekit_key_for_development_32chars_ok

GOOGLE_API_KEY=AIzaSy...你的Google密钥
```

---

## 4. 启动步骤

### 第一步：启动 LiveKit Server
终端 1：
```bash
livekit-server --config ./livekit.yaml
```
> 输出 `HTTP server listening on :7880` 且显示 `OK` 说明 SFU 启动成功。

### 第二步：启动 Gemini Voice Agent
终端 2（进入 `livekit` 目录）：
```bash
source .venv/bin/activate
python agent.py dev
```
> 控制台输出 `registered worker ... gemini-audio-agent` 说明已就绪并监听房间任务。

---

## 5. 浏览器端实测全双工语音对话

1. 打开浏览器访问官方测试台：👉 **[https://agents-playground.livekit.io/](https://agents-playground.livekit.io/)**
2. 点击右上角的 **`Settings`（设置齿轮）**：
   - **LiveKit Server URL**：填入 `ws://127.0.0.1:7880`
   - **API Key**：填入 `devkey`
   - **API Secret**：填入 `secret_livekit_key_for_development_32chars_ok`
3. 点击 **`Connect`（连接）**，允许浏览器调用麦克风。
4. **验证效果**：
   - 连接成功后，LiveKit SFU 自动派发任务给终端中运行的 `agent.py`；
   - Gemini 助手会通过语音打招呼并等待您的输入；
   - 对着麦克风用中文自然对话，可随时在助手发言过程中**随时插话打断**。
