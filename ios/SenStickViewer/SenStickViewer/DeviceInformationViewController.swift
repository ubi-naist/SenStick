//
//  DeviceInformationViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/12.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class DeviceInformationViewController : UIViewController
{
    // Properties
    
    @IBOutlet var serialNumberTextLabel: UILabel!
    @IBOutlet var manufacturerNameTextLabel: UILabel!
    @IBOutlet var hardwareRevisionTextLabel: UILabel!
    @IBOutlet var firmwareRevisionTextLabel: UILabel!
    @IBOutlet var updateFirmwareButton: UIButton!

    var device: SenStickDevice?
    
    // View controller life cycle
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        self.serialNumberTextLabel.text     = self.device?.deviceInformationService?.serialNumber
        self.manufacturerNameTextLabel.text = self.device?.deviceInformationService?.manufacturerName
        self.hardwareRevisionTextLabel.text = self.device?.deviceInformationService?.hardwareRevision
        self.firmwareRevisionTextLabel.text = self.device?.deviceInformationService?.firmwareRevision

        self.updateFirmwareButton.enabled = false
        if let firmwareRevision = self.device?.deviceInformationService?.firmwareRevision {
            // 空文字でも、ターゲットのリビジョンでもないならば、ファーム更新
            if firmwareRevision != "" && firmwareRevision != "rev1.0" {
                self.updateFirmwareButton.enabled = true
            }
        }
    }
    
    // Event handler    
    
    @IBAction func updateFirmwareButtonToutchUpInside(sender: UIButton) {
        
    }
}
