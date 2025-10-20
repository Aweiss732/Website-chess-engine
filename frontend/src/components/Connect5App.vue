<template>
  <div class="app desktop" v-if="!isMobile">
    <div class="game-container">
      <div class="left-sidebar">
        <div class="game-info">
          <div class="info-box">
            <h3>Connect 5</h3>
            <p>Get 5 in a row to win!</p>
            <div class="rules">
              <p>• Horizontal, vertical, or diagonal</p>
              <p>• First to 5 stones wins</p>
              <p>• White player starts</p>
            </div>
            <div class="quick-stats" v-if="gameState !== 'start'">
              <div class="stat-item">
                <span class="stat-label">Moves:</span>
                <span class="stat-value">{{ moveCount }}</span>
              </div>
              <div class="stat-item">
                <span class="stat-label">Turn:</span>
                <span class="stat-value">{{ currentPlayer === 'white' ? 'Your' : 'Andreas' }}</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="center-area">
        <div v-if="gameState === 'start'" class="start-screen">
          
          <div class="welcome-message">
            <h2>Connect 5</h2>
            <p>Press play to get matched against an opponent</p>
          </div>

          <PlayerCard
            name="Waiting for you to start..."
            :avatar="PlayerAvatar"
            color="black"
            class="full-width-card"
          />
          
          <div class="board-placeholder">
            <div class="placeholder-grid">
              <div v-for="i in 225" :key="i" class="placeholder-cell"></div>
            </div>
            <div class="play-overlay">
              <button @click="startMatchmaking" class="play-btn">
                Play Connect 5
              </button>
            </div>
          </div>

          <PlayerCard
            name="You"
            :avatar="PlayerAvatar"
            color="white"
            status="online"
            class="full-width-card"
          />
        </div>


        <div v-else-if="gameState === 'matching'" class="matching-screen">
          <PlayerCard
            name="Searching for Opponent..."
            :avatar="PlayerAvatar"
            color="black"
            status="searching"
            class="full-width-card"
          />

          <div class="searching-animation">
            <div class="searching-dots">
              <span></span>
              <span></span>
              <span></span>
            </div>
            <p class="searching-text">Finding opponent...</p>
            <div class="matching-stats">
              <div class="stat">
                <span class="stat-value">1</span>
                <span class="stat-label">player online</span>
              </div>
              <div class="stat">
                <span class="stat-value">∞</span>
                <span class="stat-label">elo rating</span>
              </div>
            </div>
          </div>

          <PlayerCard
            name="You"
            :avatar="PlayerAvatar"
            color="white"
            status="ready"
            class="full-width-card"
          />
        </div>

        <div v-else-if="gameState === 'game'" class="game-screen">
          <PlayerCard
            :name="currentGame.opponentName"
            :avatar="currentGame.opponentAvatar"
            color="black"
            status="online"
            class="full-width-card"
          />

          <div class="board-container">
            <Connect5Board
              :board="board"
              :current-player="currentPlayer"
              :game-over="gameOver"
              @cell-click="onCellClick"
            />
          </div>

          <PlayerCard
            name="You"
            :avatar="PlayerAvatar"
            color="white"
            status="online"
            class="full-width-card"
          />

          <div class="bottom-controls">
            <button @click="returnToStart" class="return-btn">Leave Game</button>
            <p>Moves: {{ moveCount }}</p>
          </div>
        </div>
      </div>

      <div class="right-sidebar">
        <ChatBox 
          :messages="currentGame.chatMessages" 
          @send="onUserChat" 
          :disabled="gameState !== 'game'"
          :placeholder="gameState !== 'game' ? 'Chat will be available during the game' : 'Say something...'"
        />
      </div>
    </div>

    <div v-if="showGameOverPopup" class="game-over-popup-overlay">
      <div class="game-over-popup">
        <h2>Game Over</h2>
        <p>{{ gameOverMessage }}</p>
        <div class="popup-buttons">
          <button @click="returnToStart" class="return-btn">Main Menu</button>
          <button @click="newGame" class="new-game-btn">Play Again</button>
        </div>
      </div>
    </div>
  </div>

  <!-- MOBILE LAYOUT -->
     <div class="app mobile" v-else>
    <div class="mobile-container">
      <div class="mobile-game-section">
        <div v-if="gameState === 'start'" class="mobile-start-screen">
          <div class="welcome-message">
            <h2>Connect 5</h2>
            <p>Press play to get matched against an opponent</p>
          </div>

          <PlayerCard
            name="Waiting for you to start..."
            :avatar="PlayerAvatar"
            color="black"
            class="mobile-player-top"
          />
          
          <div class="board-placeholder mobile">
            <div class="placeholder-grid">
              <div v-for="i in 225" :key="i" class="placeholder-cell"></div>
            </div>
            <div class="play-overlay">
              <button @click="startMatchmaking" class="play-btn mobile">
                Play Connect 5
              </button>
            </div>
          </div>

          <PlayerCard
            name="You"
            :avatar="PlayerAvatar"
            color="white"
            status="online"
            class="mobile-player-bottom"
          />
        </div>

        <div v-else-if="gameState === 'matching'" class="mobile-matching-screen">
          <PlayerCard
            name="Searching for Opponent..."
            :avatar="PlayerAvatar"
            color="black"
            status="searching"
            class="mobile-player-top"
          />

          <div class="searching-animation mobile">
            <div class="searching-dots">
              <span></span>
              <span></span>
              <span></span>
            </div>
            <p class="searching-text">Finding opponent...</p>
            <div class="matching-stats">
              <div class="stat">
                <span class="stat-value">1</span>
                <span class="stat-label">player online</span>
              </div>
              <div class="stat">
                <span class="stat-value">∞</span>
                <span class="stat-label">elo rating</span>
              </div>
            </div>
          </div>

          <PlayerCard
            name="You"
            :avatar="PlayerAvatar"
            color="white"
            status="ready"
            class="mobile-player-bottom"
          />
        </div>

        <div v-else-if="gameState === 'game'" class="mobile-game-screen">
          <PlayerCard
            :name="currentGame.opponentName"
            :avatar="currentGame.opponentAvatar"
            color="black"
            status="online"
            class="mobile-player-top"
          />

          <div class="mobile-board-section">
            <Connect5Board
              :board="board"
              :current-player="currentPlayer"
              :game-over="gameOver"
              @cell-click="onCellClick"
              class="mobile-board"
            />
          </div>

          <PlayerCard
            name="You"
            :avatar="PlayerAvatar"
            color="white"
            status="online"
            class="mobile-player-bottom"
          />

          <div class="mobile-chatbox">
            <ChatBox 
              :messages="currentGame.chatMessages" 
              @send="onUserChat" 
              :disabled="gameState !== 'game'"
              :placeholder="gameState !== 'game' ? 'Chat will be available during the game' : 'Message Andreas...'"
            />
          </div>

          <div class="mobile-bottom-controls">
            <button @click="returnToStart" class="return-btn mobile">Leave Game</button>
            <p>Moves: {{ moveCount }}</p>
          </div>
        </div>
      </div>

      <div class="mobile-info-box">
        <div class="info-box">
          <h3>Connect 5</h3>
          <p>Get 5 in a row to win!</p>
          <div class="rules">
            <p>• Horizontal, vertical, or diagonal</p>
            <p>• First to 5 stones wins</p>
            <p>• White player starts</p>
          </div>
          <div class="quick-stats" v-if="gameState !== 'start'">
            <div class="stat-item">
              <span class="stat-label">Moves:</span>
              <span class="stat-value">{{ moveCount }}</span>
            </div>
            <div class="stat-item">
              <span class="stat-label">Turn:</span>
              <span class="stat-value">{{ currentPlayer === 'white' ? 'Your' : 'Andreas' }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div v-if="showGameOverPopup" class="game-over-popup-overlay">
      <div class="game-over-popup">
        <h2>Game Over</h2>
        <p>{{ gameOverMessage }}</p>
        <div class="popup-buttons">
          <button @click="returnToStart" class="return-btn">Main Menu</button>
          <button @click="newGame" class="new-game-btn">Play Again</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from "vue";
import { useRouter } from "vue-router";
import axios from "axios";
import AndreasAvatar from '../assets/goodchessplayer.jpg';
import PlayerAvatar from '../assets/defaulyavatar.jpg';
import SearchingAvatar from '../assets/defaultavatar2.png';

import PlayerCard from './PlayerCard.vue';
import ChatBox from './ChatBox.vue';
import Connect5Board from './Connect5Board.vue';

const router = useRouter();
const GRID_SIZE = 15;

// Game states: 'start', 'matching', 'game'
const gameState = ref('start');

const board = ref(Array(GRID_SIZE).fill().map(() => Array(GRID_SIZE).fill(null)));
const currentPlayer = ref('white');
const gameOver = ref(false);
const winner = ref(null);
const moveCount = ref(0);

const currentGame = ref({
  chatMessages: [],
  opponentName: "Andreas",
  opponentAvatar: AndreasAvatar
});

const isMobile = ref(false);

const showGameOverPopup = ref(false);
const gameOverMessage = ref('');

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

function startMatchmaking() {
  gameState.value = 'matching';

  currentGame.value.chatMessages = [];

  setTimeout(() => {
    gameState.value = 'game';
    setTimeout(() => {
      currentGame.value.chatMessages.push({
        sender: "Andreas",
        text: "Finally! Takes so long to find opponents on this site. Let's play."
      });
    }, 3500); // message delay
  }, 5000); // matchmaking delay
}

function returnToStart() {
  gameState.value = 'start';
  resetGame();
  currentGame.value.chatMessages = [];
}

function resetGame() {
  board.value = Array(GRID_SIZE).fill().map(() => Array(GRID_SIZE).fill(null));
  currentPlayer.value = 'white';
  gameOver.value = false;
  winner.value = null;
  moveCount.value = 0;
  showGameOverPopup.value = false;
}

function onCellClick(row, col) {
  if (gameOver.value || board.value[row][col] !== null || currentPlayer.value !== 'white') return;

  board.value[row][col] = currentPlayer.value;
  moveCount.value++;

  if (checkWin(row, col)) {
    gameOver.value = true;
    winner.value = currentPlayer.value;
    gameOverMessage.value = 'Congratulations! You won!';
    showGameOverPopup.value = true;
    
    currentGame.value.chatMessages.push({
      sender: "Andreas",
      text: "You have to be cheating."
    });
    return;
  }

  if (checkDraw()) {
    gameOver.value = true;
    gameOverMessage.value = 'Game drawn!';
    showGameOverPopup.value = true;
    return;
  }

  currentPlayer.value = 'black';

  makeAiMove()
}

function boardToString() {
  let out = "";
  for (let r = 0; r < board.value.length; ++r) {
    for (let c = 0; c < board.value[r].length; ++c) {
      const cell = board.value[r][c];
      if (cell === 'white') out += 'W';
      else if (cell === 'black') out += 'B';
      else out += '.';
    }
  }
  return out;
}

async function makeAiMove() {
  if (gameOver.value) return;

  try {
    const boardStr = boardToString();
    //const res = await axios.post("http://localhost:8000/connect-five-move", {
    const res = await axios.post("/connect-five-move", {
      board: boardStr
    });

    const { row, col } = res.data;
    
    if (row !== -1 && col !== -1) {
      board.value[row][col] = 'black';
      moveCount.value++;

      if (checkWin(row, col)) {
        gameOver.value = true;
        winner.value = 'black';
        gameOverMessage.value = 'You lost! Better luck next time.';
        showGameOverPopup.value = true;
        
        return;
      }

      if (checkDraw()) {
        gameOver.value = true;
        gameOverMessage.value = 'Game drawn!';
        showGameOverPopup.value = true;
        return;
      }

      currentPlayer.value = 'white';
      
    }
  } catch (err) {
    console.error("AI move failed:", err);
  }
}

function checkWin(row, col) {
  const player = board.value[row][col];
  const directions = [
    [0, 1], [1, 0], [1, 1], [1, -1]
  ];

  for (const [dx, dy] of directions) {
    let count = 1;

    for (let i = 1; i <= 4; i++) {
      const newRow = row + dx * i;
      const newCol = col + dy * i;
      if (newRow >= 0 && newRow < GRID_SIZE && newCol >= 0 && newCol < GRID_SIZE && 
          board.value[newRow][newCol] === player) {
        count++;
      } else {
        break;
      }
    }

    for (let i = 1; i <= 4; i++) {
      const newRow = row - dx * i;
      const newCol = col - dy * i;
      if (newRow >= 0 && newRow < GRID_SIZE && newCol >= 0 && newCol < GRID_SIZE && 
          board.value[newRow][newCol] === player) {
        count++;
      } else {
        break;
      }
    }

    if (count >= 5) return true;
  }

  return false;
}

function checkDraw() {
  return board.value.flat().every(cell => cell !== null);
}

async function onUserChat(userMessage) {
  const safeMessage = escapeHtml(userMessage);
  currentGame.value.chatMessages.push({ sender: "You", text: safeMessage });

  try {
    const boardStr = boardToString();
    //const res = await axios.post("http://localhost:8000/connect-five-chat", {
    const res = await axios.post("/connect-five-chat", {
      message: safeMessage,
      board: boardStr,
      move_count: moveCount.value
    });

    const aiReply = res.data.reply || "…";
    currentGame.value.chatMessages.push({ sender: "Andreas", text: aiReply });
  } catch (err) {
    console.error("Chat failed:", err);
    currentGame.value.chatMessages.push({ sender: "Andreas", text: "I'm focusing on the game." });
  }
}

function newGame() {
  resetGame();
  showGameOverPopup.value = false;
  setTimeout(() => {
      currentGame.value.chatMessages.push({
        sender: "Andreas",
        text: "Rematch?"
      });
    }, 2500); // message delay
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
</script>

<style scoped>
@font-face {
  font-family: 'BurtonsNightmare';
  src: url('../assets/fonts/NITEMARE.TTF') format('truetype');
  font-weight: normal;
  font-style: normal;
  font-display: swap;
}

.app.desktop {
  min-height: calc(100vh - 80px);
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

.title-box{
  transform: translateY(40px);
}

.left-sidebar,
.right-sidebar {
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding-top: 36px;
}

.left-sidebar {
  flex: 0 0 200px;
}

.right-sidebar {
  flex: 0 0 350px;
  padding-left: 36px;
}

.center-area {
  width: 500px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
  padding-top: 16px;
  box-sizing: border-box;
  transform: translateX(40px)
}

.full-width-card {
  width: 100%;
}

.full-width-card :deep(.player-card) {
  width: 100%;
  max-width: none;
}

.start-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem; 
  transform: translateY(-50px);
}

.welcome-message {
  text-align: center;
  padding: 0.5rem;
  margin: 0.5rem 0;
}

.welcome-message h2 {
  font-family: 'BurtonsNightmare', cursive;
  font-size: 2.5rem;
  color: #556B2F;
  margin-bottom: 0.5rem;
  text-shadow: 2px 2px 4px rgba(139, 69, 19, 0.2);
}

.welcome-message p {
  font-size: 1.1rem;
  color: #5a4a3a;
  margin: 0;
}

.board-placeholder {
  position: relative;
  width: 100%;
  background: #DEB887;
  border: 3px solid #725334;
  border-radius: 4px;
  padding: 10px;
  box-shadow: 0 4px 12px rgba(139, 69, 19, 0.3);
  margin: 0.5rem 0;
}

.placeholder-grid {
  display: grid;
  grid-template-columns: repeat(15, 1fr);
  grid-template-rows: repeat(15, 1fr);
  gap: 1px;
  opacity: 0.3;
}

.placeholder-cell {
  aspect-ratio: 1;
  background: #c19a6b;
  border-radius: 1px;
}

.play-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(139, 69, 19, 0.1);
  border-radius: 4px;
}

.play-btn {
  background: linear-gradient(135deg, #8FBC8F 0%, #556B2F 100%);
  color: white;
  border: none;
  padding: 1rem 2rem;
  font-size: 1.5rem;
  font-weight: 600;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 6px 12px rgba(85, 107, 47, 0.4);
  font-family: 'BurtonsNightmare', cursive, sans-serif;
}

.play-btn:hover {
  transform: translateY(-3px);
  box-shadow: 0 8px 16px rgba(85, 107, 47, 0.5);
  background: linear-gradient(135deg, #9fcc9f 0%, #657b3f 100%);
}

.matching-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
}

.searching-animation {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 300px;
  gap: 2rem;
}

.searching-dots {
  display: flex;
  gap: 0.5rem;
}

.searching-dots span {
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: linear-gradient(135deg, #8FBC8F 0%, #556B2F 100%);
  animation: searchingBounce 1.4s ease-in-out infinite both;
}

.searching-dots span:nth-child(1) { animation-delay: -0.32s; }
.searching-dots span:nth-child(2) { animation-delay: -0.16s; }

@keyframes searchingBounce {
  0%, 80%, 100% {
    transform: scale(0);
  } 40% {
    transform: scale(1);
  }
}

.searching-text {
  font-size: 1.2rem;
  color: #556B2F;
  font-weight: 600;
  text-align: center;
}

.matching-stats {
  display: flex;
  gap: 2rem;
  margin-top: 1rem;
}

.stat {
  display: flex;
  flex-direction: column;
  align-items: center;
}

.stat-value {
  font-size: 1.5rem;
  font-weight: bold;
  color: #D2691E;
  font-family: 'BurtonsNightmare', cursive;
}

.stat-label {
  font-size: 0.8rem;
  color: #718096;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.game-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
}

.board-container {
  display: flex;
  align-items: flex-start;
  justify-content: center;
  gap: 1px;
  margin: 0.15rem 0;
  width: 100%;
  box-sizing: border-box;
}

.bottom-controls {
  text-align: center;
  padding: 0.75rem;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  border-radius: 8px;
  border: 1px solid #e2d6c9;
  box-shadow: 0 2px 4px rgba(0,0,0,0.06);
  width: 100%;
  margin-top: 0.5rem;
}

.return-btn {
  background: linear-gradient(135deg, #D2691E 0%, #8B4513 100%);
  color: white;
  border: none;
  padding: 0.50rem 1.1rem;
  font-size: 1.1rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(139, 69, 19, 0.3);
  margin-bottom: 0.75rem;
  width: 35%;
  /* font-family: 'BurtonsNightmare', cursive, sans-serif; */
}

.return-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(139, 69, 19, 0.4);
}

.new-game-btn {
  background: linear-gradient(135deg, #8FBC8F 0%, #556B2F 100%);
  color: white;
  border: none;
  padding: 0.50rem 1.1rem;
  font-size: 1.1rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 4px 6px rgba(85, 107, 47, 0.3);
  margin-bottom: 0.75rem;
  width: 35%;
  /* font-family: 'BurtonsNightmare', cursive, sans-serif; */
  margin-left: 0.5rem;
}

.new-game-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(85, 107, 47, 0.4);
}

.bottom-controls p {
  margin: 0.5rem 0 0 0;
  font-size: 0.9rem;
  color: #685a4a;
}

.game-info {
  width: 100%;
}

.info-box {
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  padding: 1.5rem;
  border-radius: 8px;
  border: 2px solid #8FBC8F;
  box-shadow: 0 2px 4px rgba(0,0,0,0.06);
}

.info-box h3 {
  color: #556B2F;
  margin-bottom: 0.5rem;
  font-family: 'BurtonsNightmare', cursive;
  text-align: center;
  font-size: 1.5rem;
}

.info-box p {
  color: #5a4a3a;
  margin: 0.5rem 0;
  text-align: center;
}

.rules {
  margin-top: 1rem;
  font-size: 0.9rem;
}

.rules p {
  text-align: left;
  margin: 0.25rem 0;
  font-size: 0.8rem;
}

.quick-stats {
  margin-top: 1.5rem;
  padding-top: 1rem;
  border-top: 1px solid #e2d6c9;
}

.stat-item {
  display: flex;
  justify-content: space-between;
  margin-bottom: 0.5rem;
  font-size: 0.9rem;
}

.stat-label {
  color: #5a4a3a;
  font-weight: 500;
}

.stat-value {
  color: #556B2F;
  font-weight: 600;
  font-family: 'BurtonsNightmare', cursive;
}

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
  background: linear-gradient(135deg, #FFF8DC 0%, #F5F5DC 100%);
  padding: 2rem;
  border-radius: 12px;
  border: 3px solid #8FBC8F;
  text-align: center;
  max-width: 350px;
  width: 90%;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
}

.game-over-popup h2 {
  color: #556B2F;
  margin-bottom: 1rem;
  font-size: 1.5rem;
}

.game-over-popup p {
  color: #5a4a3a;
  margin-bottom: 1.5rem;
  font-size: 1.1rem;
}

.popup-buttons {
  display: flex;
  gap: 1rem;
  justify-content: center;
}

.popup-buttons button {
  flex: 1;
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

.mobile-game-section {
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
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

.mobile-start-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
}

.mobile-start-screen .welcome-message {
  text-align: center;
  padding: 0.5rem;
  margin: 0.5rem 0;
}

.mobile-start-screen .welcome-message h2 {
  font-family: 'BurtonsNightmare', cursive;
  font-size: 2rem;
  color: #556B2F;
  margin-bottom: 0.5rem;
}

.mobile-start-screen .welcome-message p {
  font-size: 1rem;
  color: #5a4a3a;
  margin: 0;
}

.board-placeholder.mobile {
  width: 100%;
  max-width: 95vw;
  margin: 0.5rem 0;
}

.board-placeholder.mobile .placeholder-grid {
  grid-template-columns: repeat(15, 1fr);
  grid-template-rows: repeat(15, 1fr);
  gap: 1px;
}

.board-placeholder.mobile .placeholder-cell {
  aspect-ratio: 1;
}

.play-btn.mobile {
  padding: 0.8rem 1.5rem;
  font-size: 1.2rem;
}

.mobile-matching-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
}

.searching-animation.mobile {
  height: 200px;
  gap: 1.5rem;
}

.searching-animation.mobile .searching-text {
  font-size: 1.1rem;
}

.matching-stats {
  gap: 1.5rem;
}

.stat-value {
  font-size: 1.3rem;
}

.stat-label {
  font-size: 0.7rem;
}

.mobile-game-screen {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.5rem;
}

.mobile-board-section {
  display: flex;
  justify-content: center;
  margin: 0.25rem 0;
  width: 100%;
  max-width: 100vw;
  box-sizing: border-box;
  padding: 0 0.5rem;
}

.mobile-board-section :deep(.connect5-board) {
  width: 100%;
  max-width: 95vw;
  padding: 5px;
}

.mobile-board-section :deep(.board-cell) {
  width: calc(95vw / 15 - 2px);
  height: calc(95vw / 15 - 2px);
  max-width: 30px;
  max-height: 30px;
}

.mobile-board-section :deep(.white-stone .stone),
.mobile-board-section :deep(.black-stone .stone) {
  width: calc(95vw / 15 - 8px);
  height: calc(95vw / 15 - 8px);
  max-width: 28px;
  max-height: 28px;
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

.return-btn.mobile {
  background: linear-gradient(135deg, #D2691E 0%, #8B4513 100%);
  color: white;
  border: none;
  padding: 0.6rem 1rem;
  font-size: 1rem;
  font-weight: 600;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.3s ease;
  box-shadow: 0 2px 4px rgba(139, 69, 19, 0.3);
  margin-bottom: 0.5rem;
  width: 60%;
  /* font-family: 'BurtonsNightmare', cursive, sans-serif; */
}

.mobile-bottom-controls p {
  margin: 0.25rem 0 0 0;
  font-size: 0.7rem;
  color: #685a4a;
}

/* Mobile Info Box */
.mobile-info-box {
  width: 100%;
  margin-top: 0.5rem;
}

.mobile-info-box .info-box {
  padding: 1rem;
  font-size: 0.9rem;
}

.mobile-info-box .info-box h3 {
  font-size: 1.3rem;
}

.mobile-info-box .rules p {
  font-size: 0.8rem;
  margin: 0.2rem 0;
}

.mobile-info-box .stat-item {
  font-size: 0.8rem;
}

/* Mobile Game Over Popup */
@media (max-width: 768px) {
  .game-over-popup {
    padding: 1.5rem;
    max-width: 280px;
  }
  
  .game-over-popup h2 {
    font-size: 1.3rem;
  }
  
  .game-over-popup p {
    font-size: 1rem;
    margin-bottom: 1.25rem;
  }
  
  .popup-buttons {
    flex-direction: column;
    align-items: center;
    gap: 0.5rem;
  }
  
  .popup-buttons button {
    width: 80%;
    margin: 0;
  }
}

@media (max-width: 360px) {
  .app.mobile {
    padding: 0.25rem;
  }
  
  .mobile-container {
    gap: 0.4rem;
  }
  
  .mobile-player-top :deep(.player-card),
  .mobile-player-bottom :deep(.player-card) {
    padding: 0.4rem;
    font-size: 0.75rem;
  }
  
  .mobile-player-top :deep(.avatar),
  .mobile-player-bottom :deep(.avatar) {
    width: 35px;
    height: 35px;
  }
  
  .mobile-start-screen .welcome-message h2 {
    font-size: 1.7rem;
  }
  
  .mobile-start-screen .welcome-message p {
    font-size: 0.9rem;
  }
  
  .play-btn.mobile {
    padding: 0.7rem 1.2rem;
    font-size: 1.1rem;
  }
  
  .return-btn.mobile {
    width: 70%;
    font-size: 0.9rem;
    padding: 0.5rem 0.8rem;
  }
  
  .searching-animation.mobile {
    height: 180px;
    gap: 1rem;
  }
  
  .searching-animation.mobile .searching-text {
    font-size: 1rem;
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

@media (max-width: 768px) {
  .app.desktop {
    display: none;
  }
  
  .app.mobile {
    display: block;
  }
}
</style>