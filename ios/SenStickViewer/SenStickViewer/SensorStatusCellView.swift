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
    @IBOutlet var readLogButton: UIButton!
    @IBOutlet var startButton: UIButton!
    @IBOutlet var stopButton: UIButton!
    
    weak var service: SenStickControlService? {
        didSet {
            service?.delegate = self
            updateView()
        }
    }
    var name: String? {
        didSet {
            self.deviceNameTextLabel.text = name
        }
    }
    
    // Private methods
    internal func updateView() {
        if let control = self.service {
            control.delegate = self
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
    func didCommandChanged(sender: SenStickControlService, command: SenStickControlCommand)
    {
        debugPrint("\(#function)")
        updateView()
    }
    func didAvailableLogCountChanged(sender:SenStickControlService, logCount: UInt8)
    {
        debugPrint("\(#function)")
        updateView()
    }
    func didDateTimeUpdate(sender:SenStickControlService, dateTime:NSDate)
    {
        debugPrint("\(#function)")
        updateView()
    }
    func didAbstractUpdate(sender:SenStickControlService, abstractText:String)
    {
        debugPrint("\(#function)")
        updateView()
    }
    
    // MARK: - Eventhandler
    @IBAction func  startButtonToutchUpInside(sender: UIButton) {
        service?.writeCommand(.Starting)
    }
    
    @IBAction func  stopButtonToutchUpInside(sender: UIButton) {
        service?.writeCommand(.Stopping)
    }
    /*
    @IBAction func  readLogButtonToutchUpInside(sender: UIButton) {
        let logID = SensorLogID(logID:UInt8(targetLogTextInput.text!)!, skipCount: 0, position: 0)
        device?.accelerationSensorService?.writeLogID(logID)
    }
 */
/*
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
    
    @IBAction func humidityButtonToutchUpInside(sender: UIButton) {
        humidtyButton.selected = !humidtyButton.selected
        let status :SenStickStatus = humidtyButton.selected ? .SensingAndLogging : .Stopping
        device?.humiditySensorService!.writeSetting( SensorSettingData<HumiditySensorRange>(status: status, samplingDuration: SamplingDurationType(milliSeconds: 300), range: .HUMIDITY_RANGE_DEFAULT))
    }
*/
}