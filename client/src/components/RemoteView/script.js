import { mapActions } from "vuex";

export default {
    props: {
        viewId: {
          type: String,
          default: '-1',
        },
        client: {
          type: Object,
          default: null,
        },
      },
  data () {
  return {
    fullScreen:false,
    CONSOLE_EVENT:false,
    defaultWindowWidth: 800,
    defaultWindowHeight: 600,
    width:800,
    height:600,
   
    ui: {
        defaultWindowWidth: 400,
        defaultWindowHeight: 400,
        useServerAspectRatio: true,
        toggleTerminalKey: {code: "KeyT",
                            modifiers: {altKey: false,
                                        ctrlKey: true,
                                        metaKey: false,
                                        shiftKey: true}},
        toggleOutputKey: {code: "KeyO",
                          modifiers: {altKey: false,
                                      ctrlKey: true,
                                      metaKey: false,
                                      shiftKey: true}},
        consoleLayout: "vertical", //or "horizontal"
        defaultDownloadFileName: "appclient-config.txt",
        evalCode: true,
        //converts command line to js e.g. "resize 800 600" TO resize(800, 600)
        commandLineToJS: false,
        horizontalLayoutThreshold: 1000, //if image widh > threshold use horizontal
                                         //layout
      },
  }
  },
  mounted()
  { //Step.1 Obtainconfiguration
    //this.$store.dispatch('client/getAppConnection')
    this.connect();
    window.addEventListener('resize', this.getDimensions);

    //Step2. GUI setup
    //this.$store.dispatch('view/guiSetup')
    //RemoteView.$emit('guiSetup');

    
  },
  watch: {
    client() {
      this.connect();
    },
},

  actions: {
   
},  
computed: {
    /* ...mapGetters({
       counter:' getCount',
       config: ' client/CONFIG',
         
   }),*/
   //...mapState(['dataLoading'])
   getWidth()
   {
    if (this.fullScreen)
    {
      this.ui.defaultWindowWidth=this.width;
    }  else this.ui.defaultWindowWidth=this.defaultWindowWidth;
     return this.ui.defaultWindowWidth;
   },
   getHeight()
   {
    if (this.fullScreen)
    {
    this.ui.defaultWindowHeight=this.width*this.defaultWindowHeight/this.defaultWindowWidth
    } else this.ui.defaultWindowHeight=this.defaultWindowHeight;
     return this.ui.defaultWindowHeight;
   }
  },


 
methods: {
  ...mapActions({
    setImage: "client/SET_IMAGE",
    mouseDown: "client/MOUSE_DOWN",
    mouseMove: "client/MOUSE_MOVE",
    mouseUp: "client/MOUSE_UP",
    keyDown: "client/KEY_DOWN",
    keyUp: "client/KEY_UP",
  
}),

getDimensions() {
  this.width = document.documentElement.clientWidth;
  this.height = document.documentElement.clientHeight;
},
 setFullScreen()
 { 
 if (!this.fullScreen)
 {
  this.defaultWindowWidth=this.ui.defaultWindowWidth;
  this.defaultWindowHeight=this.ui.defaultWindowHeight;
  this.getDimensions()
 }
 this.fullScreen=!this.fullScreen;
 },


  mouseDown2(event)
  { 
    console.log("Mouse is Down")
    event.preventDefault()
    var ev=event;
    //var t={test:"test",l:"another"}
    //console.log(ev)
    var img=this.$refs['myid'];
    console.log(img.getBoundingClientRect())
    //this.$store.dispatch('client/MOUSE_DOWN',ev) //,{img: this.$refs['myid']});
    this.evalMouseDown(ev);
  },
    connect() {
        if (this.client) {
      
          console.log ("Ready to connect to the remote server")
          console.log(this.client)
          this.CONSOLE_EVENT = false;
   
        this.ui=this.client.ui;
        this.$store.dispatch('client/SET_IMAGE',this.$refs['myid'])//{img: this.$refs['myid']});


        }
      },





    

    },


}
