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
                service?.writeDateTime(Date())
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
            startButton.isEnabled   = (control.command == .stopping)
            stopButton.isEnabled    = (control.command != .stopping)
            storageStatusTextLabe.isHidden = !(self.service?.storageStatus)!
            if (self.service?.storageStatus)! {
                startButton.isEnabled = false
                stopButton.isEnabled  = false
            }
            formatButton.isEnabled = true
            deviceNameTextField.text = control.deviceName
        } else {
            countOfLogTextLabel.text = "0"
            startButton.isEnabled   = false
            stopButton.isEnabled    = false
            storageStatusTextLabe.isHidden = true
            formatButton.isEnabled = false
            deviceNameTextField.text = ""
        }
        deviceNameTextField.delegate = self
    }
    
    // UITextFieldDelegate
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        textField.resignFirstResponder()

        self.service?.writeDeviceName(deviceNameTextField.text!)
        
        return true
    }
    
    // MARK: - SenStickControlServiceDelegate
    func didCommandChanged(_ sender: SenStickControlService, command: SenStickControlCommand)
    {
//        debugPrint("\(#function)")
        updateView()
    }
    func didAvailableLogCountChanged(_ sender:SenStickControlService, logCount: UInt8)
    {
//        debugPrint("\(#function), \(service?.availableLogCount)")
        updateView()
    }
    func didStorageStatusChanged(_ sender:SenStickControlService, storageStatus: Bool)
    {
        updateView()
    }
    func didDateTimeUpdate(_ sender:SenStickControlService, dateTime:Date)
    {
//        debugPrint("\(#function), \(service?.dateTime)")
        updateView()
    }
    func didAbstractUpdate(_ sender:SenStickControlService, abstractText:String)
    {
//        debugPrint("\(#function)")
        updateView()
    }
    func didDeviceNameUpdate(_ sender: SenStickControlService, deviceName: String) {
        updateView()
    }
    
    // MARK: - Eventhandler
    @IBAction func  startButtonToutchUpInside(_ sender: UIButton) {
        controller?.clearGraph()
        service?.writeDateTime(Date())
        service?.writeCommand(.starting)
    }
    
    @IBAction func  stopButtonToutchUpInside(_ sender: UIButton) {
        service?.writeCommand(.stopping)
    }
    
    @IBAction func  formatButtonToutchUpInside(_ sender: UIButton) {
        service?.writeCommand(.formatStorage)
    }
}
