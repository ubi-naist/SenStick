//
//  DeviceListCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/04/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

// デバイス一覧の、セルを提供します。

class DeviceListCellView: UITableViewCell
{
    @IBOutlet var deviceNameLabel:UILabel!
    @IBOutlet var deviceUUIDLabel:UILabel!
    
    var device: SenStickDevice? {
        willSet {
            self.device?.removeObserver(self, forKeyPath: "name")
        }
        didSet {
            self.device?.addObserver(self, forKeyPath: "name", options: .new , context: nil)
            self.deviceNameLabel.text = device?.name ?? "(null)"
            self.deviceUUIDLabel.text = device?.identifier.uuidString ?? "(null)"
        }
    }
    
    deinit
    {
        self.device = nil
    }

    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?) {
        if (context == nil) {
            self.deviceNameLabel.text = device?.name
        } else {
            super.observeValue(forKeyPath: keyPath, of: object, change: change, context: context)
        }
    }
}
