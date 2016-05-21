//
//  DeviceListCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/04/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class DeviceListCellView: UITableViewCell
{
    @IBOutlet var deviceNameLabel:UILabel!
    @IBOutlet var deviceUUIDLabel:UILabel!
    
    var device: SenStickDevice? {
        willSet {
            self.device?.removeObserver(self, forKeyPath: "name")
        }
        didSet {
            self.device?.addObserver(self, forKeyPath: "name", options: .New , context: nil)
            self.deviceNameLabel.text = device?.name ?? "(null)"
            self.deviceUUIDLabel.text = device?.identifier.UUIDString ?? "(null)"
        }
    }
    
    deinit
    {
        self.device = nil
    }

    override func observeValueForKeyPath(keyPath: String?, ofObject object: AnyObject?, change: [String : AnyObject]?, context: UnsafeMutablePointer<Void>) {
        if (context == nil) {
            self.deviceNameLabel.text = device?.name
        } else {
            super.observeValueForKeyPath(keyPath, ofObject: object, change: change, context: context)
        }
    }
}
