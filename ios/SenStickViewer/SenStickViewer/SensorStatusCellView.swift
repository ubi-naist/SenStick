//
//  SensorStatusCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SensorStatusCellView: UITableViewCell , SenStickControlServiceDelegate {
    @IBOutlet var deviceNameTextLabel: UILabel!
    @IBOutlet var countOfLogTextLabel: UILabel!
    @IBOutlet var targetLogTextInput: UITextField!
    
    @IBOutlet var selectAllButton: UIButton!
    @IBOutlet var gyroButton: UIButton!
    @IBOutlet var magnetronButton: UIButton!
    @IBOutlet var humidtyButton: UIButton!
    @IBOutlet var UVButton: UIButton!
    @IBOutlet var accelerationButton: UIButton!
    @IBOutlet var barometerButton: UIButton!
    @IBOutlet var luxButton: UIButton!
    
    @IBOutlet var readLogButton: UIButton!
    @IBOutlet var startButton: UIButton!
    @IBOutlet var stopButton: UIButton!
    
    weak var device: SenStickDevice? {
        willSet {
            self.device?.controlService?.delegate = nil
        }
        didSet {
            self.device?.controlService?.delegate = self
            updateView()
        }
    }
    
    // Private methods
    internal func updateView() {
        deviceNameTextLabel.text = device?.name
        
        debugPrint("\(#function) \(device?.accelerationSensorService) command:\(device?.controlService?.command)")
        if let s = device?.accelerationSensorService {
            accelerationButton.hidden = false
            accelerationButton.enabled  = true
            accelerationButton.selected = (s.settingData?.status != .Stopping)
        } else {
            accelerationButton.hidden = true
        }
        
        if let s = device?.gyroSensorService {
            gyroButton.hidden = false
            gyroButton.enabled  = true
            gyroButton.selected = (s.settingData?.status != .Stopping)
        } else {
            gyroButton.hidden = true
        }
        
        if let s = device?.magneticFieldSensorService {
            magnetronButton.hidden = false
            magnetronButton.enabled  = true
            magnetronButton.selected = (s.settingData?.status != .Stopping)
        } else {
            magnetronButton.hidden = true
        }

        if let s = device?.brightnessSensorService {
            luxButton.hidden = false
            luxButton.enabled  = true
            luxButton.selected = (s.settingData?.status != .Stopping)
        } else {
            luxButton.hidden = true
        }
        
        if let s = device?.uvSensorService {
            UVButton.hidden = false
            UVButton.enabled  = true
            UVButton.selected = (s.settingData?.status != .Stopping)
        } else {
            UVButton.hidden = true
        }
        
        if let s = device?.humiditySensorService {
            humidtyButton.hidden = false
            humidtyButton.enabled  = true
            humidtyButton.selected = (s.settingData?.status != .Stopping)
        } else {
            humidtyButton.hidden = true
        }
        
        if let s = device?.pressureSensorService {
            barometerButton.hidden = false
            barometerButton.enabled  = true
            barometerButton.selected = (s.settingData?.status != .Stopping)
        } else {
            barometerButton.hidden = true
        }
        
        if let control = device?.controlService {
            countOfLogTextLabel.text = "\(control.availableLogCount)"
            readLogButton.enabled = (control.availableLogCount > 0)
            startButton.enabled   = (control.command == .Stopping)
            stopButton.enabled    = (control.command != .Stopping)
        } else {
            countOfLogTextLabel.text = "0"
            readLogButton.enabled = false
            startButton.enabled   = false
            stopButton.enabled    = false
        }
    }
    
    // MARK: - SenStickControlServiceDelegate
    func didCommandChanged(sender: SenStickControlService, command: SenStickControlCommand) {
        updateView()
    }
    func didAvailableLogCountChanged(sender:SenStickControlService, logCount: UInt8)
    {
        updateView()
    }
    func didDateTimeUpdate(sender:SenStickControlService, dateTime:NSDate)
    {
        updateView()
    }
    func didAbstractUpdate(sender:SenStickControlService, abstractText:String)
    {
        updateView()
    }
    
    // MARK: - Eventhandler
    @IBAction func  startButtonToutchUpInside(sender: UIButton) {
        device?.controlService?.writeCommand(.Starting)
    }
    
    @IBAction func  stopButtonToutchUpInside(sender: UIButton) {
        device?.controlService?.writeCommand(.Stopping)
    }
    
    @IBAction func  readLogButtonToutchUpInside(sender: UIButton) {
        let logID = SensorLogID(logID:UInt8(targetLogTextInput.text!)!, skipCount: 0, position: 0)
        device?.accelerationSensorService?.writeLogID(logID)
    }
    
    @IBAction func  gyroButtonToutchUpInside(sender: UIButton) {
        gyroButton.selected = !gyroButton.selected
        let status :SenStickStatus = gyroButton.selected ? .SensingAndLogging : .Stopping
        device?.gyroSensorService!.writeSetting( SensorSettingData<RotationRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .ROTATION_RANGE_250DPS) )
    }
    @IBAction func magnetronButtonToutchUpInside(sender: UIButton) {
        magnetronButton.selected = !magnetronButton.selected
        let status :SenStickStatus = magnetronButton.selected ? .SensingAndLogging : .Stopping
        device?.magneticFieldSensorService!.writeSetting(SensorSettingData<MagneticFieldRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .MAGNETIC_RANGE_DEFAULT))
    }

    @IBAction func UVButtonButtonToutchUpInside(sender: UIButton) {
        UVButton.selected = !UVButton.selected
        let status :SenStickStatus = UVButton.selected ? .SensingAndLogging : .Stopping
        device?.uvSensorService!.writeSetting( SensorSettingData<UVSensorRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .UV_RANGE_DEFAULT) )
    }
    
    @IBAction func accelerationButtonToutchUpInside(sender: UIButton) {
        accelerationButton.selected = !accelerationButton.selected
        let status :SenStickStatus = accelerationButton.selected ? .SensingAndLogging : .Stopping
        device?.accelerationSensorService!.writeSetting( SensorSettingData<AccelerationRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .ACCELERATION_RANGE_2G) )
    }
    
    @IBAction func barometerButtonToutchUpInside(sender: UIButton) {
        barometerButton.selected = !barometerButton.selected
        let status :SenStickStatus = barometerButton.selected ? .SensingAndLogging : .Stopping
        device?.pressureSensorService!.writeSetting( SensorSettingData<PressureRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .PRESSURE_RANGE_DEFAULT))
    }
    
    @IBAction func luxButtonToutchUpInside(sender: UIButton) {
        luxButton.selected = !luxButton.selected
        let status :SenStickStatus = luxButton.selected ? .SensingAndLogging : .Stopping
        device?.brightnessSensorService!.writeSetting( SensorSettingData<BrightnessRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .BRIGHTNESS_RANGE_DEFAULT))
    }
}