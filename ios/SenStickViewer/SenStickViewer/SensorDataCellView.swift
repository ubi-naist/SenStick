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
    var logid: UInt8 = 0
    
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
        self.logid = logid
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
    
    func stopReadingLog(fileName: String, duration: SamplingDurationType?)
    {
        if logData != nil && duration != nil {
            // ファイルに保存
            let folderPath = NSSearchPathForDirectoriesInDomains(.DocumentDirectory,  .UserDomainMask, true).first! as NSString
            let filePath   = folderPath.stringByAppendingPathComponent("\(fileName)_\(self.logid).csv")
        
            saveToFile(filePath, data: logData!, duration: duration!)
        }
        
        logData = nil
        self.progressBar?.hidden = true
    }
    
    func saveToFile(filePath:String, data:[[Double]], duration: SamplingDurationType)
    {
        var content = ""
        let colomn  = data.count
        let row     = data[0].count

        var time :Double = 0
        for r in 0..<row {
            content += "\(time) , "
            for c in 0..<colomn {
                if data[c].count > r {
                    content += "\((data[c])[r])"
                } else {
                    content += " , "
                    break
                }
                if c != (colomn - 1) {
                    content += " , "
                }
            }
            content += "\n"
            time    += duration.duration
        }

        do {
            try content.writeToFile(filePath, atomically: true, encoding: NSUTF8StringEncoding)
        } catch {
            debugPrint("\(#function) fatal error in file save.")
        }
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

    func didFinishedLogData(sender: AnyObject)
    {}
    // MARK: - Event handler
}
