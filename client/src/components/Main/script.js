
import { mapGetters } from 'vuex';
import ConnectionControl from '../ConnectionControl.vue'
import RemoteView from '../RemoteView/index.vue'
import { exec } from 'child_process' //will work only on server side

export default {
  name: "Main",
  components: {
    ConnectionControl,
    RemoteView
    //AirportCard, //: require("../AirportCard.vue").default
     //: require("../Connection.vue").default
  },
  setup() {
    

  },
  created(){
    //get initial data from config
    
  },
  mounted()
  { //Step.1 Obtainconfiguration
    this.$store.dispatch('client/getAppConnection')
    console.log ("obtained config")
    console.log(this.config)

    //Step2. GUI setup
    //this.$store.dispatch('view/guiSetup')
    //RemoteView.$emit('guiSetup');

    
  },
  data: () => ({ 
    loading: false,
    count:0,
    shell_command:"test",
    
    }),
  props: {
    msg: String,
   
  },
methods: {
    wsconnect(){

    },

    teststore(){
       this.$store.commit('increment')
      //console.log(this.$store.state.count)
      this.count=this.$store.state.count
      console.log(this.counter)
    },
    async connect() {
 

exec('ls -lh', (error, stdout, stderr) => {
  if (error) {
    console.error(`error: ${error.message}`);
    return;
  }

  if (stderr) {
    console.error(`stderr: ${stderr}`);
    return;
  }

  console.log(`stdout:\n${stdout}`);
});

    },
    },
    computed: {
     /* ...mapGetters({
        counter:' getCount',
        config: ' client/CONFIG',
          
    }),*/
    ...mapGetters({
      counter: "getCount",
      config: "client/CONFIG",
  }),
  },
};
