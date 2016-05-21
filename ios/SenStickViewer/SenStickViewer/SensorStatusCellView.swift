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
    func updateView() {
        deviceNameTextLabel.text = device?.name
        countOfLogTextLabel.text = "\(device?.controlService?.availableLogCount)"
        
        gyroButton.enabled         = false
        magnetronButton.enabled    = false
        temperatureButton.enabled  = false
        humidtyButton.enabled      = false
        UVButton.enabled           = false
        accelerationButton.enabled = (device?.accelerationSensorService != nil)
        barometerButton.enabled    = false
        luxButton.enabled          = false
        
        readLogButton.enabled = (device?.controlService != nil)
        startButton.enabled   = (device?.controlService != nil)
        stopButton.enabled    = (device?.controlService != nil)
    }
    
    // MARK: - SenStickControlServiceDelegate
    func didStatusChanged(sender:SenStickControlService, status:SenStickStatus)
    {
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
}