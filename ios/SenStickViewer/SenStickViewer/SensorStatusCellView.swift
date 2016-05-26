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
    @IBOutlet var deviceNameTextLabel: UILabel!
    @IBOutlet var countOfLogTextLabel: UILabel!
    @IBOutlet var targetLogTextInput: UITextField!
    @IBOutlet var readLogButton: UIButton!
    @IBOutlet var startButton: UIButton!
    @IBOutlet var stopButton: UIButton!
    
    weak var controller: SensorDataViewController?
    
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
        targetLogTextInput.delegate = self
    }
    
    // UITextFieldDelegate
    func textFieldShouldReturn(textField: UITextField) -> Bool {
        targetLogTextInput.resignFirstResponder()
        return true
    }
    
    // MARK: - SenStickControlServiceDelegate
    func didCommandChanged(sender: SenStickControlService, command: SenStickControlCommand)
    {
        debugPrint("\(#function)")
        updateView()
    }
    func didAvailableLogCountChanged(sender:SenStickControlService, logCount: UInt8)
    {
        debugPrint("\(#function), \(service?.availableLogCount)")
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

    @IBAction func  readLogButtonToutchUpInside(sender: UIButton) {
        let logid = UInt8(targetLogTextInput.text!)!
        controller?.startToReadLog(logid)
    }

    
}