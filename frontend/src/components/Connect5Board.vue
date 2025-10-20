<template>
  <div class="connect5-board">
    <div 
      v-for="(row, rowIndex) in board" 
      :key="rowIndex" 
      class="board-row"
    >
      <div
        v-for="(cell, colIndex) in row"
        :key="colIndex"
        class="board-cell"
        :class="{
          'white-stone': cell === 'white',
          'black-stone': cell === 'black',
          'game-over': gameOver
        }"
        @click="() => $emit('cell-click', rowIndex, colIndex)"
      >
        <div class="stone" v-if="cell"></div>
      </div>
    </div>
  </div>
</template>

<script setup>
defineProps({
  board: {
    type: Array,
    required: true
  },
  currentPlayer: {
    type: String,
    default: 'white'
  },
  gameOver: {
    type: Boolean,
    default: false
  }
});

defineEmits(['cell-click']);
</script>

<style scoped>
.connect5-board {
  background: #DEB887;
  border: 3px solid #725334;
  border-radius: 4px;
  padding: 10px;
  box-shadow: 0 4px 12px rgba(139, 69, 19, 0.3);
}

.board-row {
  display: flex;
}

.board-cell {
  width: 30px;
  height: 30px;
  border: 1px solid #8b5825;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  transition: background-color 0.2s ease;
  position: relative;
}

.board-cell:hover:not(.game-over) {
  background-color: rgba(210, 105, 30, 0.2);
}


.white-stone .stone {
  width: 24px;
  height: 24px;
  background: linear-gradient(135deg, #ffffff, #F0F0F0);
  border: 2px solid #C0C0C0;
  border-radius: 50%;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
}

.black-stone .stone {
  width: 24px;
  height: 24px;
  background: linear-gradient(135deg, #2F4F4F, #000000);
  border: 2px solid #1A1A1A;
  border-radius: 50%;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.5);
}

.game-over .board-cell {
  cursor: default;
}
</style>