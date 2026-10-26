const onenetConfig = require('../../config');

Page({
    data: {
      Temp:0,
      Hum:0,
      // 新增：风扇控制相关数据
      fanSpeedOptions: ['关闭', '1挡', '2挡', '3挡'], // picker 显示的选项
      fanSpeedText: '关闭' // 当前选中的档位文本，默认关闭
    },
      
    config: onenetConfig,
    Onenet_SetAlarmInfo(event)
    {
      const is_checked = event.detail.value; // 获取开关状态
      wx.showToast({
        title: '操作成功', // 提示的文字内容
        icon: 'success', // 图标类型，使用成功图标
        duration: 1000 // 提示框自动隐藏的时间，单位是毫秒
      }), 
      wx.request({
        url: this.config.setinfo_url,
        header:{
          'authorization':this.config.authorization
        },
        method:"POST",
        data: {
          "product_id": this.config.product_id,
          "device_name": this.config.device_name,
          "params": {
            "B": is_checked
          }     
        },            
      })
    },    
    Onenet_SetLedInfo(event)
    {
      const is_checked = event.detail.value; // 获取开关状态
      wx.showToast({
        title: '操作成功', // 提示的文字内容
        icon: 'success', // 图标类型，使用成功图标
        duration: 1000 // 提示框自动隐藏的时间，单位是毫秒
      }), 
      wx.request({
        url: this.config.setinfo_url,
        header:{
          'authorization':this.config.authorization
        },
        method:"POST",
        data: {
          "product_id": this.config.product_id,
          "device_name": this.config.device_name,
          "params": {
            "L": is_checked
          }     
        },            
      })
    },   
    Onenet_SetFanInfo(event) {
        console.log('picker发送选择改变，携带值为', event.detail.value);
        
        // 1. 获取选中项在数组中的索引 (0, 1, 2, 3)
        const selectedIndex = event.detail.value;
        
        // 2. 根据索引获取对应的档位值 (0, 1, 2, 3)
        const fanSpeedValue = parseInt(selectedIndex);
    
        // 3. 更新界面上显示的当前档位文本
        this.setData({
          fanSpeedText: this.data.fanSpeedOptions[selectedIndex]
        });
    
        // 4. 显示操作成功的提示
        wx.showToast({
          title: '风扇档位已设置',
          icon: 'success',
          duration: 1000
        });
    
        // 5. 向 OneNet 平台发送请求，设置风扇档位
        wx.request({
          url: this.config.setinfo_url,
          header: {
            'authorization': this.config.authorization,
            'Content-Type': 'application/json' // 建议加上这个，确保数据以JSON格式发送
          },
          method: "POST",
          data: {
            "product_id": this.config.product_id,
            "device_name": this.config.device_name,
            "params": {
              "F": fanSpeedValue // 关键点：将档位值赋给物模型标志 "F"
            }
          }
        
        })
      }
  })

  
  