<template>
  <div class="app desktop" v-if="!isMobile">
    <div class="game-container">
      <div class="left-sidebar">
        <div class="opponent-selector-container">
          <label>Play against:</label>
          <OpponentSelector
              v-model="opponent"
              :options="[
                { value: 'Andreas', name: 'Andreas', avatar: AndreasAvatar, status: 'online' },
                { value: 'python', name: 'Python Engine', avatar: PythonAvatar, status: 'engine' }
              ]"
          />
        </div>
      </div>

      <div class="center-area">
        <PlayerCard
          :name="currentGame.opponentName"
          :avatar="currentGame.opponentAvatar"
          color="black"
          :status="opponent === 'Andreas' ? 'online' : 'engine'"
        />

        <div class="board-eval-container">
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

        <PlayerCard
          name="You"
          :avatar="PlayerAvatar"
          color="white"
          status="online"
        />

        <div class="bottom-controls">
          <button @click="newGame" class="new-game-btn">New Game</button>
          <p>FEN: <code>{{ currentGame.fen }}</code></p>
        </div>
      </div>

      <div class="right-sidebar">
        <ChatBox :messages="currentGame.chatMessages" @send="onUserChat" />
      </div>
    </div>

    <div v-if="showGameOverPopup" class="game-over-popup-overlay">
      <div class="game-over-popup">
        <h2>Game Over</h2>
        <p>{{ gameOverMessage }}</p>
        <div class="popup-buttons">
          <button @click="newGame" class="new-game-btn">New Game</button>
          <a :href="githubUrl" target="_blank" class="github-link">
            Github
          </a>
        </div>
      </div>
    </div>
  </div>

  <div class="app mobile" v-else>
    <div class="mobile-container">
      <div class="mobile-opponent-selector">
        <label>Play against:</label>
        <div class="horizontal-opponents">
          <div
            v-for="option in [
              { value: 'Andreas', name: 'Andreas', avatar: AndreasAvatar, status: 'online' },
              { value: 'python', name: 'Python Engine', avatar: PythonAvatar, status: 'engine' }
            ]"
            :key="option.value"
            class="opponent-option"
            :class="{ selected: opponent === option.value }"
            @click="opponent = option.value"
          >
            <img :src="option.avatar" class="avatar" />
            <div class="opponent-info">
              <div class="name">{{ option.name }}</div>
              <div class="status" :class="option.status">
                <span class="dot"></span>
                {{ option.status === 'online' ? 'Online' : 'Engine' }}
              </div>
            </div>
          </div>
        </div>
      </div>

      <PlayerCard
        :name="currentGame.opponentName"
        :avatar="currentGame.opponentAvatar"
        color="black"
        :status="opponent === 'Andreas' ? 'online' : 'engine'"
        class="mobile-player-top"
      />

      <div class="mobile-board-section">
        <div class="board-eval-wrapper">
          <div class="board-container">
            <TheChessboard
              :board-config="boardConfig"
              player-color="white"
              @board-created="onBoardCreated"
              @move="onPlayerMove"
              class="mobile-board"
            />
          </div>
          <div class="eval-bar-container mobile">
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
        class="mobile-player-bottom"
      />

      <div class="mobile-chatbox">
        <ChatBox :messages="currentGame.chatMessages" @send="onUserChat" />
      </div>

      <div class="mobile-bottom-controls">
        <button @click="newGame" class="new-game-btn mobile">New Game</button>
        <p>FEN: <code>{{ currentGame.fen }}</code></p>
      </div>
    </div>

    <div v-if="showGameOverPopup" class="game-over-popup-overlay">
      <div class="game-over-popup">
        <h2>Game Over</h2>
        <p>{{ gameOverMessage }}</p>
        <div class="popup-buttons">
          <button @click="newGame" class="new-game-btn">New Game</button>
          <a :href="githubUrl" target="_blank" class="github-link">
            Github
          </a>
        </div>
      </div>
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

const githubUrl = computed(() => {
  return opponent.value === 'python' 
    ? 'https://github.com/Aweiss732/Website-chess-engine/blob/master/backend/engine/engine_strong.py'
    : 'https://github.com/Aweiss732/Website-chess-engine/blob/master/backend/engine/engine_core.cpp';
});

const boardConfig = ref({
  fen: currentGame.value.fen,
  orientation: "white",
  movable: {
    color: "white",
    free: false,
  },
});

const isMobile = ref(false);

const evalBarStyle = computed(() => {
  if (isMobile.value) {
    return { height: evalBarHeight.value + '%' };
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

const gameResult = computed(() => {
  const game = currentGame.value.game;

  if (game.isCheckmate()) {
    return game.turn() === 'w' ? 'lost' : 'won';
  }

  if (game.isStalemate()) {
    return 'stalemate';
  }

  if (game.isInsufficientMaterial()) {
    return 'insufficient';
  }
  
  if (game.isThreefoldRepetition()) {
    return 'repetition';
  }

  if (game.isDraw()) {
    return '50move';
  }
  
  return null;
});

const showGameOverPopup = ref(false);
const gameOverMessage = ref('');

function handleGameOver() {
  const result = gameResult.value;
  if (!result) return;
  
  switch (result) {
    case 'won':
      gameOverMessage.value = 'Congratulations! You won!';
      break;
    case 'lost':
      gameOverMessage.value = 'You lost!';
      break;
    case 'stalemate':
      gameOverMessage.value = 'Game drawn by stalemate.';
      break;
    case 'insufficient':
      gameOverMessage.value = 'Game drawn by insufficient material.';
      break;
    case 'repetition':
      gameOverMessage.value = 'Game drawn by threefold repetition.';
      break;
    case '50move':
      gameOverMessage.value = 'Game drawn by 50-move rule.';
      break;
  }
  
  showGameOverPopup.value = true;
}

watch(gameResult, (newResult) => {
  if (newResult) {
    setTimeout(handleGameOver, 2000);
  }
});

watch(() => currentGame.value.fen, () => {
  if (gameResult.value) {
    handleGameOver();
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
    //const res = await axios.post("http://localhost:8000/chat", {
    const res = await axios.post("/chat", {
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
      //? "http://localhost:8000/bestmove-python"
      //: "http://localhost:8000/bestmove";
      ? "/bestmove-python"
      : "/bestmove";

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
  showGameOverPopup.value = false;
  gameOverMessage.value = '';
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
  const bigEvalSwing = evalJump <= -250;
  const blunder = evalJump <= -1000;

  const captured = move.captured;
  const bigCapture = captured === "q" || captured === "r" || captured === "b" || captured === "n";

  if ((!bigCapture && !bigEvalSwing) && !blunder) return;

  andreasTauntCount.value++;

  try {
    //const res = await axios.post("http://localhost:8000/taunt", {
    const res = await axios.post("/taunt", {
      captured: captured ?? null,
      eval_jump: evalJump,
      fen: currentGame.value.game.fen()
    });

    const taunt = res.data.reply || "You're pieces be walking the plank, matey.";
    currentGame.value.chatMessages.push({ sender: "Andreas", text: taunt });
  } catch (err) {
    console.error("Taunt fetch failed:", err);
  }
}
</script>

<style scoped>
@font-face {
  font-family: 'BurtonsNightmare';
  src: url('../assets/fonts/NITEMARE.TTF') format('truetype');
  font-weight: normal;
  font-style: normal;
  font-display: swap;
}

/* DESKTOP STYLES */
.app.desktop {
  min-height: 100vh;
  padding: 1rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.game-container {
  display: flex;
  width: 100%;
  max-width: 1200px;
  margin: 0 auto;
  gap: 2rem;
  align-items: flex-start;
  justify-content: center;
  flex-wrap: nowrap;
}

.left-sidebar,
.right-sidebar {
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding-top: 36px;
}

.left-sidebar {
  flex: 0 0 180px;
}

.left-sidebar::after {
  content: "";
  display: block;
  width: 120px;
  height: 1px;
  visibility: hidden;
}

.right-sidebar {
  flex: 0 0 350px;
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding-top: 36px;
  padding-left: 36px;
}

.center-area {
  width: 390px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
  padding-top: 16px;
  box-sizing: border-box;
  transform: translateX(20px);
}

.center-area > .player-card {
  width: 100%;
}

.center-area > .player-card:first-of-type {
  margin-bottom: 0.4rem;
}

.center-area > .player-card:last-of-type {
  margin-top: 0.4rem;
}

.board-eval-container {
  display: flex;
  align-items: flex-start;
  justify-content: center;
  position: relative;
  top: 1px;
  gap: 1;
  margin: 0.15rem 0;
  width: 100%;
  box-sizing: border-box;
}

:deep(.cg-wrap) {
  width: 380px !important;
  height: 380px !important;
  box-sizing: border-box;
  margin: 0;
}

:deep(.cg-board) {
  width: 380px !important;
  height: 380px !important;
  box-sizing: border-box;
  margin: 0;
}

.eval-bar-container {
  width: 10px;
  height: 382px;
  background: linear-gradient(180deg, #f9d49c, #d76f00);
  display: flex;
  flex-direction: column-reverse;
  border-radius: 2px;
  overflow: hidden;
  box-shadow: inset 0 0 2px rgba(0, 0, 0, 0.2);
  margin-left: -2px;
}

.eval-bar {
  width: 100%;
  background: linear-gradient(180deg, #24a148, #145a32);
  transition: height 0.2s ease;
  box-sizing: border-box;
}

.bottom-controls {
  text-align: center;
  padding: 0.75rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  border-radius: 8px;
  border: 1px solid #e2d6c9;
  box-shadow: 0 2px 4px rgba(0,0,0,0.06);
  width: 418px;
  margin-top: 0.5rem;
  margin-bottom: 0;
}

.bottom-controls button {
  background: linear-gradient(135deg, #ff8a04 0%, #d76f00 100%);
  color: white;
  border: none;
  padding: 0.50rem 1.1rem;
  font-size: 1.275rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(215, 111, 0, 0.3);
  margin-bottom: 0.75rem;
  width: 35%;
  font-family: 'BurtonsNightmare', cursive, sans-serif;
}

.bottom-controls button:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(215, 111, 0, 0.4);
}

.bottom-controls p {
  margin: 0.5rem 0 0 0;
  font-size: 0.75rem;
  color: #685a4a;
  word-break: break-all;
}

.bottom-controls code {
  background: #ffffff;
  padding: 0.25rem 0.5rem;
  border-radius: 3px;
  font-size: 0.7rem;
  font-family: monospace;
  color: #5a4a3a;
  border: 1px solid #e2d6c9;
}

.opponent-selector-container {
  width: 100%;
}

.opponent-selector-container label {
  display: block;
  font-weight: 600;
  font-size: 1rem;
  margin-bottom: 0.75rem;
  color: #2d3748;
  text-align: center;
  padding: 0.5rem;
  background: linear-gradient(135deg, #ff8a04 0%, #422300 100%);
  color: white;
  border-radius: 6px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.opponent-selector-container :deep(.opponent-selector) {
  display: flex;
  gap: 0.75rem;
  flex-direction: column;
}

.opponent-selector-container :deep(.card) {
  padding: 0.75rem;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.3s ease;
  background: white;
  border: 2px solid #e2e8f0;
  box-shadow: 0 2px 4px rgba(0,0,0,0.08);
  text-align: center;
}

.opponent-selector-container :deep(.card:hover) {
  transform: translateY(-2px);
  box-shadow: 0 4px 8px rgba(0,0,0,0.12);
}

.opponent-selector-container :deep(.card.selected) {
  border-color: #d76f00;
  background: linear-gradient(135deg, #fff8f0 0%, #fff0e0 100%);
  box-shadow: 0 4px 12px rgba(215, 111, 0, 0.2);
}

.opponent-selector-container :deep(.avatar) {
  width: 40px;
  height: 40px;
  border-radius: 50%;
  border: 2px solid #e2e8f0;
  margin: 0 auto 0.5rem;
  transition: border-color 0.3s ease;
}

.opponent-selector-container :deep(.card.selected .avatar) {
  border-color: #d76f00;
}

.opponent-selector-container :deep(.name) {
  font-weight: 600;
  font-size: 0.8rem;
  color: #2d3748;
  margin-bottom: 0.25rem;
}

.opponent-selector-container :deep(.status) {
  font-size: 0.7rem;
  display: flex;
  align-items: center;
  justify-content: center;
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
  margin-right: 4px;
  background-color: currentColor;
}

/* Game Over Popup */
.game-over-popup-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.7);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.game-over-popup {
  background: linear-gradient(135deg, #fff8f0 0%, #fff0e0 100%);
  padding: 2rem;
  border-radius: 12px;
  border: 3px solid #d76f00;
  text-align: center;
  max-width: 300px;
  width: 90%;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
}

.game-over-popup h2 {
  color: #d76f00;
  margin-bottom: 1rem;
  font-size: 1.5rem;
}

.game-over-popup p {
  color: #5a4a3a;
  margin-bottom: 1.5rem;
  font-size: 1.1rem;
  line-height: 1.4;
}

.popup-buttons {
  display: flex;
  justify-content: center;
}

.game-over-popup .new-game-btn {
  background: linear-gradient(135deg, #ff8a04 0%, #d76f00 100%);
  color: white;
  border: none;
  padding: 0.50rem 1.1rem;
  font-size: 1.275rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(215, 111, 0, 0.3);
  font-family: 'BurtonsNightmare', cursive, sans-serif;
  width: 80%;
}

.game-over-popup .new-game-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(215, 111, 0, 0.4);
}

.popup-buttons {
  display: flex;
  justify-content: center;
  gap: 0.75rem;
  flex-wrap: wrap;
}

.github-link {
  background: linear-gradient(135deg, #333333 0%, #1a1a1a 100%);
  color: white;
  border: none;
  padding: 0.50rem 1.1rem;
  font-size: 1.1rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
  text-decoration: none;
  display: inline-block;
  text-align: center;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  margin-left: 0;
  width: 80%;
}

.github-link:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.4);
  background: linear-gradient(135deg, #444444 0%, #2a2a2a 100%);
}

/* MOBILE STYLES */
.app.mobile {
  min-height: 100vh;
  padding: 0.5rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  overflow-x: hidden;
}

.mobile-container {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
  max-width: 100%;
  margin: 0 auto;
  min-height: 100vh;
  width: 100%;
}

.mobile-opponent-selector {
  width: 100%;
  margin-bottom: 0.5rem;
}

.mobile-opponent-selector label {
  display: block;
  font-weight: 600;
  font-size: 0.8rem;
  margin-bottom: 0.5rem;
  color: #2d3748;
  text-align: center;
  padding: 0.4rem;
  background: linear-gradient(135deg, #ff8a04 0%, #422300 100%);
  color: white;
  border-radius: 6px;
  box-shadow: 0 2px 4px rgba(0,0,0,0.1);
  /* font-family: 'BurtonsNightmare', cursive, sans-serif; */
}

.horizontal-opponents {
  display: flex;
  gap: 0.5rem;
  justify-content: space-between;
  width: 100%;
}

.opponent-option {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.5rem;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.3s ease;
  background: white;
  border: 2px solid #e2e8f0;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  flex: 1;
}

.opponent-option.selected {
  border-color: #d76f00;
  background: linear-gradient(135deg, #fff8f0 0%, #fff0e0 100%);
  box-shadow: 0 2px 6px rgba(215, 111, 0, 0.2);
}

.opponent-option .avatar {
  width: 35px;
  height: 35px;
  border-radius: 50%;
  border: 2px solid #e2e8f0;
  transition: border-color 0.3s ease;
  flex-shrink: 0;
}

.opponent-option.selected .avatar {
  border-color: #d76f00;
}

.opponent-info {
  flex: 1;
  min-width: 0;
}

.opponent-option .name {
  font-weight: 600;
  font-size: 0.75rem;
  color: #2d3748;
  margin-bottom: 0.1rem;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.opponent-option .status {
  font-size: 0.65rem;
  display: flex;
  align-items: center;
  color: #718096;
}

.opponent-option .status.online {
  color: #38a169;
}

.opponent-option .status.engine {
  color: #6b7280;
}

.opponent-option .status .dot {
  width: 5px;
  height: 5px;
  border-radius: 50%;
  display: inline-block;
  margin-right: 3px;
  background-color: currentColor;
  flex-shrink: 0;
}

.mobile-player-top,
.mobile-player-bottom {
  width: 100%;
  margin: 0.25rem 0;
}

.mobile-player-top :deep(.player-card),
.mobile-player-bottom :deep(.player-card) {
  padding: 0.5rem;
  font-size: 0.8rem;
  width: 100%;
}

.mobile-player-top :deep(.avatar),
.mobile-player-bottom :deep(.avatar) {
  width: 40px;
  height: 40px;
}

.mobile-board-section {
  display: flex;
  justify-content: center;
  margin: 0.25rem 0;
  width: 100%;
}

.board-eval-wrapper {
  display: flex;
  align-items: stretch;
  gap: 2px;
  width: 100%;
  max-width: 95vw;
  margin: 0 auto;
}

.board-container {
  flex: 1;
  position: relative;
  min-width: 0;
}

.board-container :deep(.cg-wrap) {
  width: 100% !important;
  height: auto !important;
  aspect-ratio: 1 / 1 !important;
  max-width: 100% !important;
}

.board-container :deep(.cg-board) {
  width: 100% !important;
  height: 100% !important;
  position: absolute !important;
  top: 0 !important;
  left: 0 !important;
}

.eval-bar-container.mobile {
  width: 8px;
  height: 100%;
  background: linear-gradient(180deg, #f9d49c, #d76f00);
  display: flex;
  flex-direction: column-reverse;
  border-radius: 1px;
  overflow: hidden;
  box-shadow: inset 0 0 1px rgba(0, 0, 0, 0.2);
  margin: 0;
  flex-shrink: 0;
}

.eval-bar-container.mobile .eval-bar {
  width: 100%;
  background: linear-gradient(180deg, #24a148, #145a32);
  transition: height 0.2s ease;
}

.mobile-chatbox {
  width: 100%;
  margin: 0.5rem 0;
}

.mobile-chatbox :deep(.chatbox) {
  width: 100%;
}

.mobile-chatbox :deep(.messages) {
  height: 120px;
  font-size: 0.8rem;
}

.mobile-bottom-controls {
  text-align: center;
  padding: 0.5rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  border-radius: 6px;
  border: 1px solid #e2d6c9;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  width: 100%;
  margin-top: 0.25rem;
}

.mobile-bottom-controls .new-game-btn.mobile {
  background: linear-gradient(135deg, #ff8a04 0%, #d76f00 100%);
  color: white;
  border: none;
  padding: 0.6rem 1rem;
  font-size: 1rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 2px 4px rgba(215, 111, 0, 0.3);
  margin-bottom: 0.5rem;
  width: 60%;
  font-family: 'BurtonsNightmare', cursive, sans-serif;
}

.mobile-bottom-controls p {
  margin: 0.25rem 0 0 0;
  font-size: 0.7rem;
  color: #685a4a;
  word-break: break-all;
}

.mobile-bottom-controls code {
  background: #ffffff;
  padding: 0.2rem 0.4rem;
  border-radius: 3px;
  font-size: 0.65rem;
  font-family: monospace;
  color: #5a4a3a;
  border: 1px solid #e2d6c9;
}

@media (max-width: 360px) {
  .app.mobile {
    padding: 0.25rem;
  }
  
  .mobile-container {
    gap: 0.4rem;
  }
  
  .opponent-option {
    padding: 0.4rem;
    gap: 0.4rem;
  }
  
  .opponent-option .avatar {
    width: 30px;
    height: 30px;
  }
  
  .opponent-option .name {
    font-size: 0.7rem;
  }
  
  .board-eval-wrapper {
    max-width: 92vw;
  }
  
  .mobile-bottom-controls .new-game-btn.mobile {
    width: 70%;
    font-size: 0.9rem;
    padding: 0.5rem 0.8rem;
  }
  
  .mobile-player-top :deep(.player-card),
  .mobile-player-bottom :deep(.player-card) {
    padding: 0.4rem;
    font-size: 0.75rem;
  }

  .github-link {
    width: 30%;
    font-size: 0.9rem;
    padding: 0.5rem 0.8rem;
  }
  
  .mobile-player-top :deep(.avatar),
  .mobile-player-bottom :deep(.avatar) {
    width: 35px;
    height: 35px;
  }
}

@media (min-width: 361px) and (max-width: 768px) {
  .board-eval-wrapper {
    max-width: 95vw;
  }
  
  .opponent-option .name {
    font-size: 0.8rem;
  }
  
  .opponent-option .avatar {
    width: 40px;
    height: 40px;
  }
}

@media (min-width: 500px) and (max-width: 768px) {
  .opponent-option {
    padding: 0.75rem;
  }
  
  .opponent-option .name {
    font-size: 0.85rem;
  }
  
  .mobile-opponent-selector label {
    font-size: 1rem;
    padding: 0.5rem;
  }
  
  .mobile-player-top :deep(.player-card),
  .mobile-player-bottom :deep(.player-card) {
    padding: 0.75rem;
    font-size: 0.9rem;
  }
  
  .mobile-player-top :deep(.avatar),
  .mobile-player-bottom :deep(.avatar) {
    width: 45px;
    height: 45px;
  }
}

@media (max-width: 768px) {
  .app.desktop {
    display: none;
  }
  
  .app.mobile {
    display: block;
  }

  .popup-buttons {
    flex-direction: column;
    align-items: center;
    gap: 0.5rem;
  }
  
  .popup-buttons .new-game-btn,
  .popup-buttons .github-link {
    width: 80%;
    margin: 0;
  }

  .github-link {
    padding: 0.6rem 1rem;
    font-size: 0.8rem;
    margin-left: 0;
  }
  
  .game-over-popup h2 {
    font-size: 1.3rem;
  }
  
  .game-over-popup p {
    font-size: 1rem;
    margin-bottom: 1.25rem;
  }
  
  .game-over-popup .new-game-btn {
    padding: 0.6rem 1rem;
    font-size: 1rem;
    width: 70%;
  }
}

@media (min-width: 769px) {
  .app.mobile {
    display: none;
  }
  
  .app.desktop {
    display: flex;
  }
}
</style>