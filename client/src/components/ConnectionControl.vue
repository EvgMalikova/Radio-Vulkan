<template>
  <div class="console">
  <div id="console_container">
      <!--  DO CLOSE THE TAG IMMEDIATELY: IF AN EOL IS PRESENT IT WILL BE
            ADDED TO THE TEXTAREA-->
      <div id="console">
        <textarea id="term" class="terminal"></textarea>
      </div>
      <div id="output" tabindex='0'>
        <!-- DO CLOSE THE TAG IMMEDIATELY: IF AN EOL IS PRESENT IT WILL BE
             ADDED TO THE TEXTAREA -->
        <textarea id="log" readonly></textarea>
      </div>
    </div>
    <button class="button" @click="$store.dispatch('client/connect');">
     <template v-if="$store.getters['client/isConnected'] ">
            Disconnect
            </template>
            <template v-else>
            Connect
            </template>
             
             
    </button>
  </div>
</template>

<script>
export default {
  props: {
    connected: {
      type: String,
      required: true
    }
    
  },
  /*data: () => ({ 
      connected: false
      }),*/
}
</script>

<style scoped>
.console {
  border: 3px solid;
  border-radius: .5rem;
  padding: 1rem;
  margin-bottom: 1rem;
}
.button {
  background-color: #4CAF50; /* Green */
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
}


</style>
