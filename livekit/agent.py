"""
LiveKit Voice AI Agent - Google Gemini Live API
基于 LiveKit Agents 1.x 架构与 Google Gemini Live (Speech-to-Speech) 端到端全双工实时语音模型。
"""

import os
import logging
from dotenv import load_dotenv

from livekit import agents
from livekit.agents import AgentServer, AgentSession, Agent
from livekit.plugins import google, silero

# 优先读取 .env.local，其次读取 .env
load_dotenv(".env.local")
load_dotenv(".env")

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("gemini-audio-agent")

# 检查 Google API Key
google_api_key = os.getenv("GOOGLE_API_KEY") or os.getenv("GEMINI_API_KEY")
if not google_api_key:
    logger.warning("未检测到 GOOGLE_API_KEY 或 GEMINI_API_KEY 环境变量，请在 .env.local 或终端中配置！")

# 实例化语音助手角色定义
class GeminiVoiceAssistant(Agent):
    def __init__(self) -> None:
        super().__init__(
            instructions=(
                "你是一个友善、专业且反应迅速的 AI 语音助手。"
                "请使用自然口语化的中文与用户交流，回答要精炼简洁，适合语音实时播报。"
            )
        )

# 创建 LiveKit Agent 服务
server = AgentServer()

@server.rtc_session(agent_name="gemini-audio-agent")
async def rtc_session(ctx: agents.JobContext):
    logger.info(f"正在接入房间: {ctx.room.name} ...")

    # 1. 配置 Gemini Live 端到端实时语音模型 (S2S: Speech-to-Speech)
    # 支持模型包括: gemini-2.5-flash-native-audio-preview-12-2025, gemini-3.8-live 等
    gemini_model = os.getenv("GEMINI_MODEL", "gemini-2.5-flash-native-audio-preview-12-2025")
    gemini_voice = os.getenv("GEMINI_VOICE", "Puck")  # 可选声音: Puck, Charon, Kore, Fenrir, Aoede

    realtime_model = google.realtime.RealtimeModel(
        model=gemini_model,
        api_key=google_api_key,
        voice=gemini_voice,
        modalities=["AUDIO"],
        temperature=0.7,
    )

    # 2. 本地 Silero VAD (用于端侧低延迟打断判定与语音活性检测)
    vad = silero.VAD.load()

    # 3. 组装全双工 AgentSession
    session = AgentSession(
        llm=realtime_model,
        vad=vad,
    )

    # 4. 启动 Session 并绑定到当前 WebRTC 房间
    assistant = GeminiVoiceAssistant()
    await session.start(
        room=ctx.room,
        agent=assistant,
    )

    logger.info("Gemini Voice Agent 已成功启动，准备就绪！")

    # 5. 主动向用户打招呼
    await session.generate_reply(
        instructions="用简短的一句话（中文）向刚进房间的用户打个招呼，告诉他你是 Gemini 语音助手并询问需要什么帮助。"
    )

if __name__ == "__main__":
    agents.cli.run_app(server)
