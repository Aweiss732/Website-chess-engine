from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import chess
from engine.engine_strong import get_best_move as get_best_move_python
from engine.engine_strong_cpp import get_best_move
from engine.engine_connect5 import get_best_move as get_best_move_connect5
import os
import requests
from typing import Optional

from dotenv import load_dotenv
load_dotenv()

OPENROUTER_KEY = os.environ.get("OPENROUTER_KEY")
if not OPENROUTER_KEY:
    print("WARNING: OPENROUTER_KEY not set; /chat will return 500")

MODEL_CANDIDATES = [
    "deepseek/deepseek-chat-v3.1:free",
    "google/gemini-2.0-flash-exp:free",
    "tngtech/deepseek-r1t2-chimera:free",
    "z-ai/glm-4.5-air:free",
    "qwen/qwen3-235b-a22b:free",
]


app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:5173"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class Request(BaseModel):
    fen: str
    time_ms: int = 200


@app.post("/bestmove-python")
def bestmove(req: Request):
    result = get_best_move_python(req.fen, req.time_ms)
    return result

@app.post("/bestmove")
def bestmove(req: Request):
    result = get_best_move(req.fen, req.time_ms)
    return result

class ChatRequest(BaseModel):
    message: str
    fen: Optional[str] = None
    eval_cp: Optional[int] = None
    persona: Optional[str] = "Andreas"

class ChatResponse(BaseModel):
    reply: str

def get_prompt(persona: str, eval_ctx: str) -> str:
    if persona == "python":
        return (
            "You are a humble, logical chess tutor named PythonEngine. "
            "Interpret pronouns this way: When they say 'my'/'me', they mean themself/their own. When they say 'you' that refers to you as the system. "
            "Reply in exactly one short sentence (no more than 20 words)."
            "You provide brief, helpful, but very general advice. DO not speak about what is happening on the board. "
            "Speak like you were born rich in the 1700s london"
            f"Here is the current evalutation: {eval_ctx}. If score is greater than 0 you are losing. If score is less negative you are winning."
        )
    else:
        # Andreas persona
        return (
        "You are a cocky and very skilled chess player playing against a human" 
        "Interpret pronouns this way: When they say 'my'/'me', they mean themself/their own. When they say 'you' that refers to you as the system. "
        "Reply in exactly one short sentence (no more than 20 words)."
        "Do not mention the word count. Just send out a message like how you would say it in a normal conversation"
        "Be VERY COCKY and VERY confident. Be offensive and insulting."
        "Do not be specific in whats happening on the chess board."
        "Talk like a you are from the 1700s carribean. You love rum and like to talk about rum, Use methators with rum. Call the opponent things such as 'matey' or 'landlubber'"
        f"Here is the current evalutation: {eval_ctx}. If score is greater than 0 you are losing. If score is less negative you are winning."
    )


@app.post("/chat", response_model=ChatResponse)
def chat(req: ChatRequest):
    if not OPENROUTER_KEY:
        raise HTTPException(status_code=500, detail="Chat backend not configured")

    eval_ctx = ""
    if req.eval_cp is not None:
        pawns = req.eval_cp / 100.0
        eval_ctx = f"Position evaluation (positive = White ahead): {pawns:+.2f}. "

    system_prompt = get_prompt(req.persona, eval_ctx)

    headers = {
        "Authorization": f"Bearer {OPENROUTER_KEY}"
    }

    MODEL_CANDIDATES = [
        "deepseek/deepseek-chat-v3.1:free",
        "google/gemini-2.0-flash-exp:free",
        "tngtech/deepseek-r1t2-chimera:free",
        "z-ai/glm-4.5-air:free",
        "qwen/qwen3-235b-a22b:free",
    ]

    for model_name in MODEL_CANDIDATES:
        payload = {
            "model": model_name,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": req.message}
            ],
            "max_tokens": 40,
            "temperature": 0.8
        }

        try:
            r = requests.post("https://openrouter.ai/api/v1/chat/completions", json=payload, headers=headers, timeout=10)
            r.raise_for_status()
            data = r.json()

            choices = data.get("choices")
            if not choices or not choices[0].get("message", {}).get("content"):
                continue  # try next model

            content = choices[0]["message"]["content"].strip()
            content = content.replace("<|begin▁of▁sentence|>", "").replace("<｜begin▁of▁sentence｜>", "").strip()
            if "\n" in content:
                content = content.split("\n")[0].strip()
            if len(content.split()) > 30:
                content = "Nice try — but let's keep it short."

            print(f"[CHAT] Model used: {model_name}")
            return {"reply": content}

        except requests.RequestException as e:
            print(f"[CHAT] Model failed: {model_name} — {e}")

    raise HTTPException(status_code=502, detail="All language models failed to respond.")


class TauntRequest(BaseModel):
    captured: Optional[str] = None
    eval_jump: Optional[float] = None
    fen: Optional[str] = None

@app.post("/taunt", response_model=ChatResponse)
def taunt(req: TauntRequest):
    if not OPENROUTER_KEY:
        raise HTTPException(status_code=500, detail="Chat backend not configured")

    context_parts = []
    if req.captured:
        piece_names = {'q': 'queen', 'r': 'rook', 'b': 'bishop', 'n': 'knight', 'p': 'pawn'}
        piece = piece_names.get(req.captured.lower(), 'piece')
        context_parts.append(f"You just captured the opponent's {piece}.")
    if req.eval_jump and req.eval_jump <= -150:
        context_parts.append(f"The opponent blundered. The chess evaluation shifted greatly in your favor.")

    context = " ".join(context_parts) or "You are playing extremely well."

    system_prompt = (
        "You are a skilled chess player, talking trash to your opponent.\n"
        "Speak like you're from the 1700s Caribbean. \n"
        "Be VERY COCKY and VERY confident. Be offensive and insulting."
        "Use metaphors. Call the opponent things like 'matey', 'landlubber', or 'scallywag'.\n"
        "Make it short (under 20 words). Be reactive to, and mention the situation described.\n"
        f"Context: {context}"
        "IMPORTANT: Do NOT mention the word count."
    )

    headers = {
        "Authorization": f"Bearer {OPENROUTER_KEY}"
    }

    MODEL_CANDIDATES = [
        "deepseek/deepseek-chat-v3.1:free",
        "google/gemini-2.0-flash-exp:free",
        "tngtech/deepseek-r1t2-chimera:free",
        "z-ai/glm-4.5-air:free",
        "qwen/qwen3-235b-a22b:free",
    ]

    for model_name in MODEL_CANDIDATES:
        payload = {
            "model": model_name,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": "Say something cocky and taunting about the current situation."}
            ],
            "max_tokens": 40,
            "temperature": 0.9
        }

        try:
            r = requests.post("https://openrouter.ai/api/v1/chat/completions", json=payload, headers=headers, timeout=10)
            r.raise_for_status()
            data = r.json()

            choices = data.get("choices")
            if not choices or not choices[0].get("message", {}).get("content"):
                continue

            content = choices[0]["message"]["content"].strip()
            content = content.replace("<|begin▁of▁sentence|>", "").replace("<｜begin▁of▁sentence｜>", "").strip()
            if "\n" in content:
                content = content.split("\n")[0].strip()

            print(f"[TAUNT] Model used: {model_name}")
            return {"reply": content}

        except requests.RequestException as e:
            print(f"[TAUNT] Model failed: {model_name} — {e}")

    raise HTTPException(status_code=502, detail="All language models failed to provide a taunt.")



class ConnectFiveMoveRequest(BaseModel):
    board: str
    time_ms: int = 1000

@app.post("/connect-five-move")
async def connect_five_move(req: ConnectFiveMoveRequest):
    if not isinstance(req.board, str) or len(req.board) < 225:
        return {"row": -1, "col": -1}


    result = get_best_move_connect5(req.board[:225], req.time_ms)
    #print(result)
    return {"row": result["row"], "col": result["col"], "eval": result.get("eval", 0)}


class ConnectFiveChatRequest(BaseModel):
    message: str
    board: str
    move_count: int

@app.post("/connect-five-chat")
def connect_five_chat(req: ConnectFiveChatRequest):
    if not OPENROUTER_KEY:
        raise HTTPException(status_code=500, detail="Chat backend not configured")
    system_prompt = (
        "You are Andreas, a skilled Connect Five player. "
        "Speak like a brittish pub lad: casual british banter"
        "Be VERY COCKY and confident. "
        "Keep responses under 20 words. "
        f"Current game: {req.move_count} moves played."
    )
    
    headers = {
        "Authorization": f"Bearer {OPENROUTER_KEY}"
    }

    MODEL_CANDIDATES = [
        "deepseek/deepseek-chat-v3.1:free",
        "google/gemini-2.0-flash-exp:free",
        "tngtech/deepseek-r1t2-chimera:free",
        "z-ai/glm-4.5-air:free",
        "qwen/qwen3-235b-a22b:free",
    ]

    for model_name in MODEL_CANDIDATES:
        payload = {
            "model": model_name,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": req.message}
            ],
            "max_tokens": 40,
            "temperature": 0.8
        }

        try:
            r = requests.post("https://openrouter.ai/api/v1/chat/completions", json=payload, headers=headers, timeout=10)
            r.raise_for_status()
            data = r.json()

            choices = data.get("choices")
            if not choices or not choices[0].get("message", {}).get("content"):
                continue  # try next model

            content = choices[0]["message"]["content"].strip()
            content = content.replace("<|begin▁of▁sentence|>", "").replace("<｜begin▁of▁sentence｜>", "").strip()
            if "\n" in content:
                content = content.split("\n")[0].strip()
            if len(content.split()) > 30:
                content = "Nice try — but let's keep it short."

            print(f"[CHAT] Model used: {model_name}")
            return {"reply": content}

        except requests.RequestException as e:
            print(f"[CHAT] Model failed: {model_name} — {e}")

    raise HTTPException(status_code=502, detail="All language models failed to respond.")