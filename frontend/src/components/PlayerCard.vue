<template>
  <div class="player-card" :class="color">
    <img :src="avatar" class="avatar" />
    <div class="info">
      <div class="name">
        {{ name }}
        <span v-if="status" class="status-indicator" :class="status">
          <span class="dot"></span>
          {{ statusText }}
        </span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed } from "vue";

const props = defineProps({
  name: String,
  avatar: String,
  color: String,
  status: String // online, engine, searching, ready, or null
});

const statusText = computed(() => {
  switch (props.status) {
    case 'online': return 'Online';
    case 'engine': return 'Engine';
    case 'searching': return 'Searching';
    case 'ready': return 'Ready';
    default: return '';
  }
});
</script>

<style scoped>
.player-card {
  display: flex;
  align-items: center;
  gap: 12px;
  font-weight: bold;
  padding: 0.75rem;
  font-size: 0.75rem;
  border-radius: 12px;
  border: 1px solid #e2d6c9;
  box-shadow: 0 2px 8px rgba(0,0,0,0.06);
}

.avatar {
  width: 45px;
  height: 45px;
  border-radius: 50%;
  border: 2px solid #e2d6c9;
}

.white {
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  color: #5a4a3a;
}

.black {
  background: linear-gradient(135deg, #5a4a3a 0%, #3a2e20 100%);
  color: white;
}

.status-indicator {
  font-size: 0.8em;
  margin-left: 8px;
  display: inline-flex;
  align-items: center;
  font-weight: normal;
}

.status-indicator .dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  display: inline-block;
  margin-right: 6px;
}

.status-indicator.online .dot {
  background-color: #38a169;
}

.status-indicator.engine .dot {
  background-color: #797571;
}

.status-indicator.searching .dot {
  background-color: #d69e2e;
  animation: pulse 1.5s infinite;
}

.status-indicator.ready .dot {
  background-color: #38a169;
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}
</style>