//
//  SensorDataCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

public class SensorDataCellView: UITableViewCell , SenStickSensorServiceDelegate {
    @IBOutlet var titleTextLabel: UILabel?
    @IBOutlet var iconButton:     UIButton?
    @IBOutlet var maxTextLabel:   UILabel?
    @IBOutlet var minTextLabel:   UILabel?
    @IBOutlet var progressBar:    UIProgressView?
    @IBOutlet var graphView:      DataGraphView?
    
    var isLogging: Bool = false
    
    // MARK: - Properties
    
    var maxValue: Double {
        didSet {
            maxTextLabel?.text = String(maxValue)
        }
    }
    var minValue: Double {
        didSet {
            minTextLabel?.text = String(maxValue)
        }
    }

    // MARK: - Initializer
    public required init?(coder aDecoder: NSCoder)
    {
        maxValue = 1.0
        minValue = 0
        
        super.init(coder:aDecoder)
        
        self.iconButton?.enabled = false
        self.maxTextLabel?.text  = ""
        self.minTextLabel?.text  = ""
        self.progressBar?.hidden = true
    }
    
    func updateView() {
    }
    
    // MARK: - SenStickSensorServiceDelegate
    public func didUpdateSetting(sender:AnyObject)
    {}
    
    public func didUpdateRealTimeData(sender: AnyObject)
    {}
    
    public func didUpdateMetaData(sender: AnyObject)
    {}
    
    public func didUpdateLogData(sender: AnyObject)
    {}

    // MARK: - Event handler
}

