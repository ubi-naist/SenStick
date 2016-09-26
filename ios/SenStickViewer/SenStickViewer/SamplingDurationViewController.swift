//
//  File.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/27.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SamplingDurationViewController : UIViewController, UITextFieldDelegate, UIPickerViewDelegate, UIPickerViewDataSource
{
    @IBOutlet var durationField: UITextField!
    @IBOutlet var picker: UIPickerView!
    
    var target: AnyObject?
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        self.durationField.delegate = self
        self.picker.delegate = self
        self.picker.dataSource = self
        
        if let service = target as? AccelerationSensorService {
            if let duration = service.settingData?.samplingDuration {
                durationField.text = "\(Int(duration.duration * 1000))"
            }
        }
        
        if let service = target as? GyroSensorService {
            if let duration = service.settingData?.samplingDuration {
                durationField.text = "\(Int(duration.duration * 1000))"
            }
        }
        
        if let service = target as? MagneticFieldSensorService {
            if let duration = service.settingData?.samplingDuration {
                durationField.text = "\(Int(duration.duration * 1000))"
            }
        }
        
        if let service = target as? BrightnessSensorService {
            picker.isHidden = true
            if let duration = service.settingData?.samplingDuration {
                durationField.text = "\(Int(duration.duration * 1000))"
            }
        }
        
        if let service = target as? UVSensorService {
            picker.isHidden = true
            if let duration = service.settingData?.samplingDuration {
                durationField.text = "\(Int(duration.duration * 1000))"
            }
        }
        
        if let service = target as? HumiditySensorService {
            picker.isHidden = true
            if let duration = service.settingData?.samplingDuration {
                durationField.text = "\(Int(duration.duration * 1000))"
            }
        }
        
        if let service = target as? PressureSensorService {
            picker.isHidden = true
            if let duration = service.settingData?.samplingDuration {
                durationField.text = "\(Int(duration.duration * 1000))"
            }
        }
        
        // キーボードのDoneツールバーを追加
        let toolBar   = UIToolbar(frame: CGRect(x: 0, y: 0, width: self.view.frame.size.width, height: 50))
        toolBar.items = [UIBarButtonItem(barButtonSystemItem: .flexibleSpace , target: nil, action: nil),
                         UIBarButtonItem(barButtonSystemItem: .done, target: self, action: #selector(doneToolBarButton))]
        self.durationField.inputAccessoryView = toolBar
        toolBar .sizeToFit()
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        if let service = target as? AccelerationSensorService {
            if let value = Int(durationField.text!) {
                let sd = SamplingDurationType(milliSeconds: UInt16(value))
                let range = AccelerationRange(rawValue: UInt16(picker.selectedRow(inComponent: 0)))
                let setting = SensorSettingData<AccelerationRange>(status: (service.settingData?.status)!, samplingDuration: sd, range: range!)
                service.writeSetting(setting)
                service.readSetting()
            }
        }
        
        if let service = target as? GyroSensorService {
            if let value = Int(durationField.text!) {
                let sd = SamplingDurationType(milliSeconds: UInt16(value))
                let range = RotationRange(rawValue: UInt16(picker.selectedRow(inComponent: 0)))
                let setting = SensorSettingData<RotationRange>(status: (service.settingData?.status)!, samplingDuration: sd, range: range!)
                service.writeSetting(setting)
                service.readSetting()
            }
        }
        
        if let service = target as? MagneticFieldSensorService {
            if let value = Int(durationField.text!) {
                let sd = SamplingDurationType(milliSeconds: UInt16(value))
                let setting = SensorSettingData<MagneticFieldRange>(status: (service.settingData?.status)!, samplingDuration: sd, range: (service.settingData?.range)!)
                service.writeSetting(setting)
                service.readSetting()
            }
        }
        
        if let service = target as? BrightnessSensorService {
            if let value = Int(durationField.text!) {
                let sd = SamplingDurationType(milliSeconds: UInt16(value))
                let setting = SensorSettingData<BrightnessRange>(status: (service.settingData?.status)!, samplingDuration: sd, range: (service.settingData?.range)!)
                service.writeSetting(setting)
                service.readSetting()
            }
        }
        
        if let service = target as? UVSensorService {
            if let value = Int(durationField.text!) {
                let sd = SamplingDurationType(milliSeconds: UInt16(value))
                let setting = SensorSettingData<UVSensorRange>(status: (service.settingData?.status)!, samplingDuration: sd, range: (service.settingData?.range)!)
                service.writeSetting(setting)
                service.readSetting()
            }
            
            
        }
        
        if let service = target as? HumiditySensorService {
            if let value = Int(durationField.text!) {
                let sd = SamplingDurationType(milliSeconds: UInt16(value))
                let setting = SensorSettingData<HumiditySensorRange>(status: (service.settingData?.status)!, samplingDuration: sd, range: (service.settingData?.range)!)
                service.writeSetting(setting)
                service.readSetting()
            }
        }
        
        if let service = target as? PressureSensorService {
            if let value = Int(durationField.text!) {
                let sd = SamplingDurationType(milliSeconds: UInt16(value))
                let setting = SensorSettingData<PressureRange>(status: (service.settingData?.status)!, samplingDuration: sd, range: (service.settingData?.range)!)
                service.writeSetting(setting)
                service.readSetting()
            }
        }
    }
    
    // UITextFieldDelegate
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        durationField.resignFirstResponder()
        return true
    }
    
    func doneToolBarButton() {
        durationField.resignFirstResponder()
    }
    
    // UIPickerViewDataSource
    func numberOfComponents(in pickerView: UIPickerView) -> Int
    {
        return 1
    }
    func pickerView(_ pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int
    {
        // FIXME クラスインスタンスを見てswitch構文を使うべき
        if target is AccelerationSensorService {
            return 4
        }
        if target is GyroSensorService {
            return 4
        }
        
        // else
        return 0
    }
    
    // UIPickerViewDataSource
    func pickerView(_ pickerView: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String?
    {
        // FIXME クラスインスタンスを見てswitch構文を使うべき
        if self.target is AccelerationSensorService {
            return ["2G", "4G", "8G", "16G"][row]
        }
        if target is GyroSensorService {
            return ["250DPS", "500DPS", "1000DPS", "2000DPS"][row]
        }
        
        // else
        return nil
    }
}


/*
 if let service = target as? AccelerationSensorService {
 
 }
 
 if let service = target as? GyroSensorService {
 
 }
 if let service = target as? MagneticFieldSensorService {
 
 }
 if let service = target as? BrightnessSensorService {
 
 }
 if let service = target as? UVSensorService {
 
 }
 if let service = target as? HumiditySensorService {
 
 }
 if let service = target as? PressureSensorService {
 
 }*/
