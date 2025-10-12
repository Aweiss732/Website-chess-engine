<template>
  <div class="app">
    <div class="mobile-opponent-selector" v-if="isMobile">
      <div class="opponent-selector-container">
        <label>Play against:</label>
        <OpponentSelector
            v-model="opponent"
            :options="[
                {
                value: 'Andreas',
                name: 'Andreas',
                avatar: AndreasAvatar,
                status: 'online'
                },
                {
                value: 'python',
                name: 'Python Engine',
                avatar: PythonAvatar,
                status: 'engine'
                }
            ]"
        />
      </div>
    </div>

    <PlayerCard
      :name="currentGame.opponentName"
      :avatar="currentGame.opponentAvatar"
      color="black"
      :status="opponent === 'Andreas' ? 'online' : 'engine'"
    />

    <div class="board-area">
      <!-- Desktop: Opponent selector stays in board area -->
      <div class="desktop-opponent-selector" v-if="!isMobile">
        <div class="opponent-selector-container">
          <label>Play against:</label>
          <OpponentSelector
              v-model="opponent"
              :options="[
                  {
                  value: 'Andreas',
                  name: 'Andreas',
                  avatar: AndreasAvatar,
                  status: 'online'
                  },
                  {
                  value: 'python',
                  name: 'Python Engine',
                  avatar: PythonAvatar,
                  status: 'engine'
                  }
              ]"
          />
        </div>
      </div>

      <div class="board-wrapper">
        <TheChessboard
          :board-config="boardConfig"
          player-color="white"
          @board-created="onBoardCreated"
          @move="onPlayerMove"
        />

        <div class="eval-bar-container">
          <div
            class="eval-bar"
            :style="evalBarStyle"
            :title="evalText"
          ></div>
        </div>
      </div>
    </div>

    <PlayerCard
      name="You"
      :avatar="PlayerAvatar"
      color="white"
      status="online"
    />

    <ChatBox :messages="currentGame.chatMessages" @send="onUserChat" />

    <div class="controls">
      <button @click="newGame">New Game</button>
      <p>FEN: <code>{{ currentGame.fen }}</code></p>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch, onMounted, onUnmounted } from "vue";
import { Chess } from "chess.js";
import { TheChessboard } from "vue3-chessboard";
import axios from "axios";
import AndreasAvatar from '../assets/goodchessplayer.jpg';
import PlayerAvatar from '../assets/defaulyavatar.jpg';
import PythonAvatar from '../assets/pythonimage.png';

import PlayerCard from './PlayerCard.vue'
import ChatBox from './ChatBox.vue'
import OpponentSelector from './OpponentSelector.vue'

const opponent = ref("Andreas");
const lastEvalCP = ref(null);
const andreasTauntCount = ref(0);
const MAX_TAUNTS = 3;


const games = ref({
  Andreas: {
    game: new Chess(),
    fen: (new Chess()).fen(),
    evaluation: null,
    chatMessages: [{ sender: "Andreas", text: "Good luck." }],
    opponentName: "Andreas",
    opponentAvatar: AndreasAvatar
  },
  python: {
    game: new Chess(),
    fen: (new Chess()).fen(),
    evaluation: null,
    chatMessages: [{ sender: "Python Engine", text: "Good luck." }],
    opponentName: "Python Engine",
    opponentAvatar: PythonAvatar
  }
});

const currentGame = computed(() => games.value[opponent.value]);

const boardConfig = ref({
  fen: currentGame.value.fen,
  orientation: "white",
  movable: {
    color: "white",
    free: false,
  },
});

//-------------------------------------------------------
const isMobile = ref(false);

const evalBarStyle = computed(() => {
  if (isMobile.value) {
    return { width: evalBarHeight.value + '%' };
  } else {
    return { height: evalBarHeight.value + '%' };
  }
});

onMounted(() => {
  const checkMobile = () => {
    isMobile.value = window.innerWidth <= 768;
  };
  
  checkMobile();
  window.addEventListener('resize', checkMobile);
  
  onUnmounted(() => {
    window.removeEventListener('resize', checkMobile);
  });
});
//-------------------------------------------------------

let boardApi = null;
let ignoreNextMoveEvent = false;

const evaluation = computed(() => currentGame.value.evaluation);

const evalBarHeight = computed(() => {
  if (!evaluation.value || (!evaluation.value.cp && !evaluation.value.mate)) return 50;

  let centipawns = evaluation.value.cp ?? (evaluation.value.mate > 0 ? 10000 : -10000);

  centipawns = -centipawns;
  
  const normalized = Math.max(-1000, Math.min(1000, centipawns));
  return ((normalized + 1000) / 2000) * 100;
});

const evalText = computed(() => {
  if (!evaluation.value) return "Even";

  let score, mate;
  if (evaluation.value.mate !== null) {
    mate = -evaluation.value.mate;
    return `Mate in ${Math.abs(mate)}` + (mate > 0 ? " (You)" : " (Opponent)");
  } else {
    score = -(evaluation.value.cp / 100);
    return (score > 0 ? "+" : "") + score.toFixed(2);
  }
});

watch(opponent, (newOpponent) => {
  if (boardApi) {
    boardApi.setPosition(currentGame.value.fen);
    boardConfig.value.fen = currentGame.value.fen;
  }
});

function onBoardCreated(api) {
  boardApi = api;
  boardApi.resetBoard();
  boardApi.move(currentGame.value.fen);
}

async function onUserChat(userMessage) {
  const safeMessage = escapeHtml(userMessage);
  currentGame.value.chatMessages.push({ sender: "You", text: safeMessage });

  let cp = currentGame.value.evaluation?.cp ?? null;

  const sender = opponent.value === "python" ? "Python Engine" : "Andreas";

  try {
    const res = await axios.post("http://localhost:8000/chat", {
      message: safeMessage,
      fen: currentGame.value.game.fen(),
      eval_cp: cp,
      persona: opponent.value === "python" ? "python" : "pirate"
    });

    const aiReply = res.data.reply || "…";
    currentGame.value.chatMessages.push({ sender: sender, text: aiReply });
  } catch (err) {
    console.error("Chat failed:", err);
    currentGame.value.chatMessages.push({ sender: sender, text: "I'm focusing on the game." });
  }
}

async function onPlayerMove(moveEvent) {
  if (ignoreNextMoveEvent) {
    ignoreNextMoveEvent = false;
    return;
  }

  const move = {
    from: moveEvent.from,
    to: moveEvent.to,
  };
  if (moveEvent.promotion) move.promotion = moveEvent.promotion;

  const mv = currentGame.value.game.move(move);
  if (!mv) {
    boardApi.undo();
    return;
  }

  currentGame.value.fen = currentGame.value.game.fen();
  const uciMove = mv.from + mv.to + (mv.promotion || '');
  boardApi.move(uciMove);

  setTimeout(makeAiMove, 100);
}

async function makeAiMove() {
  if (currentGame.value.game.turn() !== "b") return;

  try {
    const url = opponent.value === "python"
      ? "http://localhost:8000/bestmove-python"
      : "http://localhost:8000/bestmove";

    const res = await axios.post(url, {
      fen: currentGame.value.game.fen(),
      time_ms: 200,
    });

    const best = res.data.bestmove;
    const score_cp = res.data.cp;
    const mate = res.data.mate;

    if (!best) return;

    const mv = currentGame.value.game.move(best);
    if (!mv) {
      console.error("Invalid AI move from chess.js:", best);
      return;
    }

    currentGame.value.fen = currentGame.value.game.fen();
    currentGame.value.evaluation = {
      cp: score_cp,
      mate: mate,
    };

    if (opponent.value === "Andreas") {
      maybeTaunt(score_cp, mv);
    }

    ignoreNextMoveEvent = true;
    const success = boardApi.move(best);
    if (!success) {
      console.error("boardApi.move failed for AI move", best);
    }
  } catch (err) {
    console.error("AI move failed:", err);
  }
}

async function newGame() {
  currentGame.value.game = new Chess();
  currentGame.value.fen = currentGame.value.game.fen();
  currentGame.value.evaluation = null;
  
  const sender = opponent.value === "python" ? "Python Engine" : "Andreas";
  currentGame.value.chatMessages = [{ sender: sender, text: "Good luck." }];

  if (boardApi) {
    boardApi.setPosition(currentGame.value.fen);
  }

  ignoreNextMoveEvent = false;
}

function escapeHtml(str) {
  return str.replace(/[&<>"']/g, function (m) {
    return ({
      '&': '&amp;',
      '<': '&lt;',
      '>': '&gt;',
      '"': '&quot;',
      "'": '&#039;',
    })[m];
  });
}

async function maybeTaunt(currentCP, move) {
  if (andreasTauntCount.value >= MAX_TAUNTS) return;

  const lastCP = lastEvalCP.value;
  lastEvalCP.value = currentCP;

  const evalJump = lastCP !== null ? currentCP - lastCP : 0;
  const bigEvalSwing = evalJump <= -150;

  const captured = move.captured;
  const bigCapture = captured === "q" || captured === "r" || captured === "b" || captured === "n";

  if (!bigCapture && !bigEvalSwing) return;

  andreasTauntCount.value++;

  try {
    const res = await axios.post("localhost:8000/taunt", {
      captured: captured ?? null,
      eval_jump: evalJump,
      fen: currentGame.value.game.fen()
    });

    const taunt = res.data.reply || "You're sinking fast, matey.";
    currentGame.value.chatMessages.push({ sender: "Andreas", text: taunt });
  } catch (err) {
    console.error("Taunt fetch failed:", err);
  }
}


</script>
<style scoped>
.app {
  max-width: 375px;
  margin: auto;
  display: flex;
  flex-direction: column;
  gap: 1.125rem;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  font-size: 0.75rem;
  padding: 1rem;
  box-sizing: border-box;
}

.app > * {
  width: 100%;
}

/* Mobile design */
@media (max-width: 768px) {
  .app {
    max-width: 100%;
    padding: 0.5rem;
    gap: 1rem;
  }
  
  .mobile-opponent-selector {
    width: 100%;
    order: -1;
  }
  
  .desktop-opponent-selector {
    display: none;
  }
  
  .board-area {
    flex-direction: column;
    padding-right: 0;
    margin-left: 0;
    gap: 1rem;
  }
  
  .opponent-selector-container {
    width: 100%;
    margin-top: 0;
  }
  
  .board-wrapper {
    flex-direction: column;
    align-items: center;
  }
  
  .eval-bar-container {
    width: 100%;
    height: 12px;
    margin-left: 0;
    margin-top: 6px;
    flex-direction: row;
  }
  
  .eval-bar {
    height: 100%;
  }
  
  .controls {
    padding: 0.5rem;
  }
  
  .controls button {
    padding: 0.75rem 1.5rem;
    font-size: 0.875rem;
  }

  .opponent-selector-container :deep(.card) {
    padding: 1rem;
  }
  
  .chat-input button,
  .controls button {
    min-height: 44px;
  }
  
  .player-card {
    padding: 1rem;
  }
}

/* Desktop styles */
@media (min-width: 769px) {
  .mobile-opponent-selector {
    display: none;
  }
  
  .desktop-opponent-selector {
    display: block;
  }
  
  .board-area {
    display: flex;
    align-items: flex-start;
    gap: 1.5rem;
    padding-right: 165px;
    margin-left: -165px;
  }
  
  .opponent-selector-container {
    width: 150px;
    flex-shrink: 0;
    margin-top: 0.75rem;
  }
  
  .board-wrapper {
    display: flex;
    flex-direction: row;
    align-items: stretch;
    position: relative;
    width: 100%;
  }
  
  .eval-bar-container {
    width: 10.5px;
    background: linear-gradient(180deg, #f9d49c, #d76f00);
    display: flex;
    flex-direction: column-reverse;
    border-radius: 4.5px;
    overflow: hidden;
    margin-left: 6px;
    box-shadow: inset 0 0 3px rgba(0, 0, 0, 0.2);
  }
  
  .eval-bar {
    width: 100%;
    background: linear-gradient(180deg, #24a148, #145a32);
    transition: height 0.3s ease;
  }
}

.opponent-selector-container label {
  display: block;
  font-weight: 600;
  font-size: 0.75rem;
  margin-bottom: 0.5625rem;
  color: #2d3748;
  text-align: center;
  padding: 0.375rem;
  background: linear-gradient(135deg, #ff8a04 0%, #422300 100%);
  color: white;
  border-radius: 6px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.opponent-selector-container :deep(.opponent-selector) {
  gap: 0.9375rem;
}

.opponent-selector-container :deep(.card) {
  padding: 0.75rem;
  border-radius: 9px;
  cursor: pointer;
  transition: all 0.3s ease;
  background: white;
  border: 2px solid #e2e8f0;
  box-shadow: 0 2px 8px rgba(0,0,0,0.08);
}

.opponent-selector-container :deep(.card:hover) {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0,0,0,0.15);
  border-color: #cbd5e0;
}

.opponent-selector-container :deep(.card.selected) {
  border-color: #d76f00;
  background: linear-gradient(135deg, #fff8f0 0%, #fff0e0 100%);
  box-shadow: 0 4px 12px rgba(215, 111, 0, 0.2);
}

.opponent-selector-container :deep(.avatar) {
  width: 45px;
  height: 45px;
  border-radius: 50%;
  border: 3px solid #e2e8f0;
  transition: border-color 0.3s ease;
}

.opponent-selector-container :deep(.card.selected .avatar) {
  border-color: #d76f00;
}

.opponent-selector-container :deep(.name) {
  font-weight: 600;
  font-size: 0.825rem;
  color: #2d3748;
}

.opponent-selector-container :deep(.status) {
  font-size: 0.675rem;
  display: flex;
  align-items: center;
  color: #718096;
}

.opponent-selector-container :deep(.status.online) {
  color: #38a169;
}

.opponent-selector-container :deep(.status.engine) {
  color: #6b7280;
}

.opponent-selector-container :deep(.status .dot) {
  width: 6px;
  height: 6px;
  border-radius: 50%;
  display: inline-block;
  margin-right: 4.5px;
  background-color: currentColor;
}

:deep(.cg-wrap) {
  max-width: 100%;
  aspect-ratio: 1;
}

:deep(.cg-board) {
  max-width: 100%;
  height: auto;
}

.chatbox {
  width: calc(100% + 24px);
  max-width: none;
}

.controls {
  text-align: center;
  padding: 0.75rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  border-radius: 9px;
  border: 1px solid #e2d6c9;
  box-shadow: 0 2px 8px rgba(0,0,0,0.06);
}

.controls button {
  background: linear-gradient(135deg, #ff8a04 0%, #d76f00 100%);
  color: white;
  border: none;
  padding: 0.5625rem 1.125rem;
  font-size: 0.75rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(215, 111, 0, 0.3);
  margin-bottom: 0.75rem;
}

.controls button:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(215, 111, 0, 0.4);
}

.controls button:active {
  transform: translateY(0);
}

.controls p {
  margin: 0.375rem 0 0 0;
  font-size: 0.675rem;
  font-family: Georgia;
  color: #685a4a;
}

.controls code {
  background: #ffffff;
  padding: 0.1875rem 0.375rem;
  border-radius: 3px;
  font-size: 0.6375rem;
  font-family: monospace;
  color: #5a4a3a;
  border: 1px solid #e2d6c9;
}
</style>