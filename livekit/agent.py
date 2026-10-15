"""
LiveKit Voice AI Agent 本地测试脚本
适用环境: macOS (M4 Max) / Linux 本地部署
支持后端: LM Studio (默认监听 http://127.0.0.1:1234/v1)
"""

import asyncio
import os
import logging
from livekit import agents, rtc
from livekit.agents import JobContext, WorkerOptions, cli
from livekit.plugins import silero, openai

# 配置日志输出
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("local-agent")

# 本地 LM Studio 配置参数
LM_STUDIO_URL = os.getenv("LM_STUDIO_URL", "http://127.0.0.1:1234/v1")
LM_STUDIO_MODEL = os.getenv("LM_STUDIO_MODEL", "qwen2.5-14b-instruct")

async def entrypoint(ctx: JobContext):
    logger.info(f"正在加入房间: {ctx.room.name} ...")
    await ctx.connect()
    logger.info("已成功接入 LiveKit SFU 媒体房间！")

    # 1. 加载本地 Silero VAD (用于毫秒级语音活性检测与打断判决)
    vad = silero.VAD.load()

    # 2. 对接本地运行的 LM Studio 作为核心对话大脑 (流式 Token 响应)
    llm = openai.LLM(
        base_url=LM_STUDIO_URL,
        api_key="lm-studio",                  # 本地无鉴权，填任意占位符
        model=LM_STUDIO_MODEL,
        temperature=0.7,
    )

    # 3. 语音识别 (STT) 与 语音合成 (TTS)
    # 模式 A (推荐混合体验): 如果配置了云端 Key，可取消注释使用商业流式:
    # from livekit.plugins import deepgram, cartesia
    # stt = deepgram.STT()
    # tts = cartesia.TTS()

    # 模式 B (全本地模式): 依赖本地 OpenAI 兼容的 STT/TTS 接口服务 (如 whisper.cpp / kokoro-fastapi)
    stt_base_url = os.getenv("STT_BASE_URL", "http://127.0.0.1:8000/v1")
    tts_base_url = os.getenv("TTS_BASE_URL", "http://127.0.0.1:8001/v1")

    stt = openai.STT(base_url=stt_base_url, api_key="dummy")
    tts = openai.TTS(base_url=tts_base_url, api_key="dummy", model="kokoro")

    # 4. 初始化全双工语音助手 (支持实时打断与低延迟断句)
    agent = agents.VoicePipelineAgent(
        vad=vad,
        stt=stt,
        llm=llm,
        tts=tts,
        allow_interruptions=True,     # 启用实时打断 (Barge-in)
        min_endpointing_delay=0.2,     # 200ms 静音判定结束 (极致优化首句响应延迟)
    )

    # 5. 绑定到房间并启动
    agent.start(ctx.room)
    logger.info("Voice Pipeline Agent 已启动，等待用户音频下发...")

    # 发送第一句欢迎语
    await agent.say("你好！我是运行在本地 LM Studio 的 AI 语音助手，请问有什么可以帮您？")

if __name__ == "__main__":
    # 启动工作进程并监听 LiveKit 房间任务分发
    cli.run_app(WorkerOptions(entrypoint_fnc=entrypoint))
