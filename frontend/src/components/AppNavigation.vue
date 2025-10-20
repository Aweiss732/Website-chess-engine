<template>
  <nav class="app-navigation">
    <div class="nav-container">
      <div class="nav-logo">
        <span class="logo-text">Andreas Weiss</span>
      </div>
      <div class="nav-links">
        <router-link 
          to="/chess" 
          class="nav-link"
          :class="{ active: $route.path === '/chess' }"
        >
          <span class="nav-icon">♛</span>
          <span class="nav-text">Chess</span>
        </router-link>
        <router-link 
          to="/connect5" 
          class="nav-link"
          :class="{ active: $route.path === '/connect5' }"
        >
          <span class="nav-icon">⍉</span>
          <span class="nav-text">Connect 5</span>
        </router-link>
        <a 
          href="https://github.com/Aweiss732/Website-chess-engine" 
          target="_blank" 
          class="nav-link github-link"
        >
          <span class="nav-text">GitHub</span>
        </a>
      </div>
    </div>
  </nav>
</template>

<script setup>

</script>

<style scoped>
.app-navigation {
  background: linear-gradient(135deg, #718d40 0%, #2f430d 100%);
  padding: 0.5rem 1rem;
  border-bottom: 3px solid #3a2e20;
  box-shadow: 0 2px 10px rgba(139, 69, 19, 0.3);
}

.nav-container {
  max-width: 1200px;
  margin: 0 auto;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.nav-logo .logo-text {
  font-family: 'BurtonsNightmare', cursive;
  font-size: 2rem;
  color: #FFF8DC;
  text-shadow: 2px 2px 4px rgba(139, 69, 19, 0.5);
}

.nav-links {
  display: flex;
  gap: 2rem;
}

.nav-link {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.75rem 1.5rem;
  background: linear-gradient(135deg, #d48b3c 0%, #d76f00 100%);
  color: white;
  text-decoration: none;
  border-radius: 8px;
  border: 2px solid #f1a24e;
  font-family: 'BurtonsNightmare', cursive;
  font-size: 1.2rem;
  transition: all 0.3s ease;
  box-shadow: 0 4px 8px rgba(139, 69, 19, 0.3);
}

.nav-link:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 12px rgba(139, 69, 19, 0.4);
  background: linear-gradient(135deg, #FFB347 0%, #E3782E 100%);
}

.nav-link.active {
  background: linear-gradient(135deg, #354b0f 0%, #556c27 100%);
  border-color: #99b992;
  box-shadow: 0 4px 12px rgba(255, 165, 0, 0.4);
}

.nav-icon {
  font-size: 1.5rem;
}

.nav-text {
  font-weight: bold;
}

.github-link {
  background: linear-gradient(135deg, #333333 0%, #1a1a1a 100%) !important;
  border-color: #555555 !important;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3) !important;
}

.github-link:hover {
  background: linear-gradient(135deg, #444444 0%, #2a2a2a 100%) !important;
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.4) !important;
  transform: translateY(-2px);
}

/* Mobile styles */
@media (max-width: 768px) {
  .nav-container {
    flex-direction: column;
    gap: 1rem;
  }
  
  .nav-logo .logo-text {
    font-size: 1.5rem;
  }
  
  .nav-links {
    gap: 1rem;
  }
  
  .nav-link {
    padding: 0.5rem 1rem;
    font-size: 1rem;
  }
  
  .nav-icon {
    font-size: 1.2rem;
  }

  .github-link {
    display: none;
  }
}
</style>