const onenetConfig = require('../../config');

Page({
  data: {
    W:0,
    S:0,
    G:0,
    fengshan:0
  },
    
  config: onenetConfig,
  Onenet_GetInfo()
  {
    wx.request({
      url: this.config.getinfo_url,
      header:{
        'authorization':this.config.authorization
      },
      method:"GET",
      success: (e) =>
      {
          console.log(e),
          this.setData({
          W:e.data.data[5].value,
          S:e.data.data[4].value,
          G:e.data.data[2].value

        })
      }
    })
  },
  
  onLoad(){
    setInterval(this.Onenet_GetInfo,5000)
  }
})
