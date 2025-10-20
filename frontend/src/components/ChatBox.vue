<template>
  <div class="chatbox">
    <div class="messages" ref="messagesContainer">
      <div
        v-for="(msg, i) in messages"
        :key="i"
        :class="['message', msg.sender === 'You' ? 'you' : 'ai']"
      >
        <div class="message-content">
          <strong>{{ msg.sender }}:</strong>
          <span v-html="escapeHtml(msg.text)"></span>
        </div>
      </div>
      <div v-if="messages.length === 0" class="no-messages">
        {{ emptyStateMessage }}
      </div>
    </div>
    <form @submit.prevent="handleSend" class="chat-input">
      <input 
        v-model="input" 
        type="text" 
        :placeholder="placeholder" 
        :disabled="disabled"
        :class="{ 'disabled': disabled }"
      />
      <button type="submit" :disabled="disabled">Send</button>
    </form>
  </div>
</template>

<script setup>
import { ref, watch, nextTick, computed } from "vue";

const props = defineProps({
  messages: Array,
  disabled: {
    type: Boolean,
    default: false
  },
  placeholder: {
    type: String,
    default: "Say something..."
  }
});

const emit = defineEmits(["send"]);

const input = ref("");
const messagesContainer = ref(null);

const emptyStateMessage = computed(() => {
  return props.disabled 
    ? "Chat will be available during the game" 
    : "No messages yet. Start the conversation!";
});

watch(() => props.messages, async () => {
  await nextTick();
  if (messagesContainer.value) {
    messagesContainer.value.scrollTop = messagesContainer.value.scrollHeight;
  }
}, { deep: true, immediate: true });

function handleSend() {
  if (!input.value.trim() || props.disabled) return;
  emit("send", input.value.trim());
  input.value = "";
}

function escapeHtml(unsafe) {
  return unsafe
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}
</script>

<style scoped>
.chatbox {
  width: 100%;
  max-width: 100%;
  border-radius: 9px;
  overflow: hidden;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  border: 1px solid #e2d6c9;
  background: linear-gradient(135deg, #f8f6f4 0%, #f0ebe6 100%);
  position: relative;
}

.chatbox::before {
  content: '';
  position: absolute;
  top: -4px;
  right: -4px;
  width: 12px;
  height: 12px;
  background: #ff4444;
  border-radius: 50%;
  opacity: 0;
  transition: opacity 0.3s ease;
  z-index: 10;
  box-shadow: 0 0 4px rgba(255, 68, 68, 0.5);
}

.chatbox.new-message::before {
  opacity: 1;
  animation: pulse 1.5s infinite;
}

@keyframes pulse {
  0% { transform: scale(1); }
  50% { transform: scale(1.2); }
  100% { transform: scale(1); }
}

.messages {
  height: 200px;
  overflow-y: auto;
  padding: 0.70rem;
  display: flex;
  flex-direction: column;
  gap: 0.5625rem;
  scroll-behavior: smooth;
  font-size: 0.8rem;
}

.messages::-webkit-scrollbar {
  width: 6px;
}

.messages::-webkit-scrollbar-track {
  background: #f1f1f1;
  border-radius: 3px;
}

.messages::-webkit-scrollbar-thumb {
  background: #d76f00;
  border-radius: 3px;
}

.messages::-webkit-scrollbar-thumb:hover {
  background: #b35900;
}

.message {
  display: flex;
  max-width: 100%;
}

.message.ai {
  align-self: flex-start;
}

.message.you {
  align-self: flex-end;
}

.message-content {
  padding: 0.4625rem 0.75rem;
  border-radius: 9px;
  word-wrap: break-word;
  box-shadow: 0 1px 3px rgba(0,0,0,0.1);
  animation: fadeIn 0.3s ease;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(5px); }
  to { opacity: 1; transform: translateY(0); }
}

.message.ai .message-content {
  background: linear-gradient(135deg, #ffffff 0%, #f8f6f4 100%);
  border: 1px solid #e2d6c9;
  color: #5a4a3a;
}

.message.you .message-content {
  background: linear-gradient(135deg, #ff8a04 0%, #d76f00 100%);
  color: white;
  border: none;
}

.message strong {
  display: block;
  margin-bottom: 0.25rem;
  font-size: 0.675rem;
  opacity: 0.9;
}

.no-messages {
  text-align: center;
  color: #a0aec0;
  font-style: italic;
  padding: 2rem 1rem;
  font-size: 0.9rem;
}

.chat-input {
  display: flex;
  padding: 0.75rem;
  gap: 0.375rem;
  background: white;
  border-top: 1px solid #e2d6c9;
}

.chat-input input {
  flex: 1;
  border: 1px solid #e2d6c9;
  border-radius: 8px;
  padding: 0.5625rem;
  font-size: 0.75rem;
  outline: none;
  transition: border-color 0.3s ease;
  background: #f8f6f4;
}

.chat-input input:focus {
  border-color: #d76f00;
  box-shadow: 0 0 0 3px rgba(215, 111, 0, 0.1);
}

.chat-input input.disabled {
  background: #f5f5f5;
  color: #a0aec0;
  cursor: not-allowed;
  opacity: 0.7;
}

.chat-input input.disabled::placeholder {
  color: #a0aec0;
}

.chat-input button {
  background: linear-gradient(135deg, #ff8a04 0%, #d76f00 100%);
  color: white;
  border: none;
  padding: 0.5625rem 1.125rem;
  border-radius: 8px;
  cursor: pointer;
  font-weight: 600;
  transition: all 0.3s ease;
  box-shadow: 0 2px 4px rgba(215, 111, 0, 0.3);
}

.chat-input button:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 4px 8px rgba(215, 111, 0, 0.4);
}

.chat-input button:disabled {
  background: linear-gradient(135deg, #cccccc 0%, #999999 100%);
  cursor: not-allowed;
  transform: none;
  box-shadow: none;
  opacity: 0.7;
}

@media (max-width: 768px) {
  .chatbox {
    width: 100%;
    margin-top: 0.5rem;
  }
  
  .messages {
    height: 120px;
    padding: 0.5rem;
    font-size: 0.8rem;
  }
  
  .message {
    max-width: 90%;
  }
  
  .chat-input {
    padding: 0.5rem;
  }
  
  .chat-input input {
    font-size: 16px;
  }
}
</style>