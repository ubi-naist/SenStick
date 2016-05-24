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
    @IBOutlet var temperatureButton: UIButton!
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
        
        temperatureButton.enabled  = false
        humidtyButton.enabled      = false
        barometerButton.enabled    = false
        
        debugPrint("\(#function) \(device?.accelerationSensorService) command:\(device?.controlService?.command)")
        if let s = device?.accelerationSensorService {
            accelerationButton.enabled  = true
            accelerationButton.selected = (s.settingData?.status != .Stopping)
        } else {
            accelerationButton.enabled  = false
            accelerationButton.selected = false
        }
        
        if let s = device?.gyroSensorService {
            gyroButton.enabled  = true
            gyroButton.selected = (s.settingData?.status != .Stopping)
        } else {
            gyroButton.enabled  = false
            gyroButton.selected = false
        }
        
        if let s = device?.magneticFieldSensorService {
            magnetronButton.enabled  = true
            magnetronButton.selected = (s.settingData?.status != .Stopping)
        } else {
            magnetronButton.enabled  = false
            magnetronButton.selected = false
        }

        if let s = device?.brightnessSensorService {
            luxButton.enabled  = true
            luxButton.selected = (s.settingData?.status != .Stopping)
        } else {
            luxButton.enabled  = false
            luxButton.selected = false
        }
        
        if let s = device?.uvSensorService {
            UVButton.enabled  = true
            UVButton.selected = (s.settingData?.status != .Stopping)
        } else {
            UVButton.enabled  = false
            UVButton.selected = false
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
        
    }
    @IBAction func magnetronButtonToutchUpInside(sender: UIButton) {
        
    }
    @IBAction func temperatureButtonToutchUpInside(sender: UIButton) {
        
    }
    @IBAction func UVButtonButtonToutchUpInside(sender: UIButton) {
        
    }
    
    @IBAction func accelerationButtonToutchUpInside(sender: UIButton) {
        accelerationButton.selected = !accelerationButton.selected
        let status :SenStickStatus = accelerationButton.selected ? .SensingAndLogging : .Stopping
        device?.accelerationSensorService!.writeSetting( SensorSettingData<AccelerationRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .ACCELERATION_RANGE_2G) )
    }
    
    @IBAction func barometerButtonToutchUpInside(sender: UIButton) {
        
    }
    @IBAction func luxButtonToutchUpInside(sender: UIButton) {
        
    }
}