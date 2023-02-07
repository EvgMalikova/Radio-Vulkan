<template>
 
  <Main msg="Radio Vulkan client" />
</template>

<script>
import Main from "./components/Main";



export default {
  name: "App",
  components: {
    Main,
  },
  data: function() {
    return {
      connection: null,
      wsImage: "ws://localhost:8881",
      wsCommand: "ws://localhost:8882"
    }
    },
  created: function() {
    

  }
};
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>
