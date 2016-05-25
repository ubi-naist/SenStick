//
//  SensorDataCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SensorDataCellView: UITableViewCell , SenStickSensorServiceDelegate {
    @IBOutlet var titleTextLabel: UILabel?
    @IBOutlet var iconButton:     UIButton?
    @IBOutlet var maxTextLabel:   UILabel?
    @IBOutlet var minTextLabel:   UILabel?
    @IBOutlet var progressBar:    UIProgressView?
    @IBOutlet var graphView:      DataGraphView?
    
    var logData: [[Double]]?
    
    // MARK: - Properties
    var maxValue: Double {
        didSet {
            maxTextLabel?.text = String(maxValue)
        }
    }
    var minValue: Double {
        didSet {
            minTextLabel?.text = String(minValue)
        }
    }

    // MARK: - Initializer
    required init?(coder aDecoder: NSCoder)
    {
        maxValue = 1.0
        minValue = 0
        
        super.init(coder:aDecoder)
        
        self.iconButton?.enabled = false
        self.maxTextLabel?.text  = ""
        self.minTextLabel?.text  = ""
        self.progressBar?.hidden = true
        
        debugPrint("\(#function), \(self.superview)")
    }
    
    func drawRealTimeData(data: [Double])
    {
        // ログ読み出し中は無効化
        if logData != nil {
            return
        }
        
        self.graphView?.plotDataArray(data)
    }
    
    func startToReadLog(logid: UInt8)
    {
        logData = [[], [], []]
        self.progressBar?.hidden = false
    }
    
    func addReadLog(data:[Double], progress: Double) {
        if logData == nil {
            return
        }

        for (index, d) in data.enumerate() {
            logData![index].append(d)
        }
        
        self.graphView?.plotLogData(logData!, percent: progress)
        self.progressBar?.progress = Float(progress)
        
//        debugPrint("\(#function), \(progress),  \(logData)")
    }
    
    func stopReadingLog(fileName: String)
    {
        // ファイルに保存
        // 終了
        logData = nil
        self.progressBar?.hidden = true
    }
    
    // MARK: - SenStickSensorServiceDelegate
     func didUpdateSetting(sender:AnyObject)
    {}
    
     func didUpdateRealTimeData(sender: AnyObject)
    {}
    
     func didUpdateMetaData(sender: AnyObject)
    {}
    
     func didUpdateLogData(sender: AnyObject)
    {}

    // MARK: - Event handler
}

