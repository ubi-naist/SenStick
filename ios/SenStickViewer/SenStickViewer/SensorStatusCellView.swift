//
//  SensorStatusCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SensorStatusCellView: UITableViewCell , SenStickControlServiceDelegate, UITextFieldDelegate {
    @IBOutlet var deviceNameTextField: UITextField!
    @IBOutlet var countOfLogTextLabel: UILabel!
    @IBOutlet var startButton: UIButton!
    @IBOutlet var stopButton: UIButton!    
    @IBOutlet var storageStatusTextLabe: UILabel!
    @IBOutlet var formatButton: UIButton!
    
    var shouldSetDateTime: Bool = true
    
    weak var controller: SensorDataViewController?
    
    weak var service: SenStickControlService? {
        didSet {
            service?.delegate = self
            updateView()
        
            if service == nil {
                shouldSetDateTime = true
            } else if shouldSetDateTime {
                service?.writeDateTime(NSDate())
                shouldSetDateTime = false
            }
        }
    }

    var name: String? {
        didSet {
            self.deviceNameTextField.text = name
        }
    }
    
    // Private methods
    internal func updateView() {
        if let control = self.service {
            control.delegate = self
            countOfLogTextLabel.text = "\(control.availableLogCount)"
            startButton.enabled   = (control.command == .Stopping)
            stopButton.enabled    = (control.command != .Stopping)
            storageStatusTextLabe.hidden = !(self.service?.storageStatus)!
            if (self.service?.storageStatus)! {
                startButton.enabled = false
                stopButton.enabled  = false
            }
            formatButton.enabled = true
            deviceNameTextField.text = control.deviceName
        } else {
            countOfLogTextLabel.text = "0"
            startButton.enabled   = false
            stopButton.enabled    = false
            storageStatusTextLabe.hidden = true
            formatButton.enabled = false
            deviceNameTextField.text = ""
        }
        deviceNameTextField.delegate = self
    }
    
    // UITextFieldDelegate
    func textFieldShouldReturn(textField: UITextField) -> Bool {
        textField.resignFirstResponder()

        self.service?.writeDeviceName(deviceNameTextField.text!)
        
        return true
    }
    
    // MARK: - SenStickControlServiceDelegate
    func didCommandChanged(sender: SenStickControlService, command: SenStickControlCommand)
    {
//        debugPrint("\(#function)")
        updateView()
    }
    func didAvailableLogCountChanged(sender:SenStickControlService, logCount: UInt8)
    {
//        debugPrint("\(#function), \(service?.availableLogCount)")
        updateView()
    }
    func didStorageStatusChanged(sender:SenStickControlService, storageStatus: Bool)
    {
        updateView()
    }
    func didDateTimeUpdate(sender:SenStickControlService, dateTime:NSDate)
    {
//        debugPrint("\(#function), \(service?.dateTime)")
        updateView()
    }
    func didAbstractUpdate(sender:SenStickControlService, abstractText:String)
    {
//        debugPrint("\(#function)")
        updateView()
    }
    func didDeviceNameUpdate(sender: SenStickControlService, deviceName: String) {
        updateView()
    }
    
    // MARK: - Eventhandler
    @IBAction func  startButtonToutchUpInside(sender: UIButton) {
        controller?.clearGraph()
        service?.writeCommand(.Starting)
    }
    
    @IBAction func  stopButtonToutchUpInside(sender: UIButton) {
        service?.writeCommand(.Stopping)
    }
    
    @IBAction func  formatButtonToutchUpInside(sender: UIButton) {
        service?.writeCommand(.FormatStorage)
    }
}
