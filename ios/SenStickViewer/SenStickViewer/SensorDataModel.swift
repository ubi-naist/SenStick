//
//  SensorDataModel.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SensorDataModel: SenStickSensorServiceDelegate {
    
    let sampleCount: Int = 300
    
    var cell: SensorDataCellView? {
        willSet {
            cell?.iconButton?.removeTarget(self, action: #selector(iconButtonToutchUpInside), forControlEvents: .TouchUpInside)
        }
        didSet {
            cell?.iconButton?.addTarget(self, action: #selector(iconButtonToutchUpInside), forControlEvents: .TouchUpInside)
            didUpdateSetting(self)
            cell?.graphView?.plotData(logData)
        }
    }
    
    var logData: [[Double]] = [[], [], []]
    var logid: UInt8 = 0
    
    // MARK: - Properties
    
    var maxValue: Double {
        didSet {
            updateCell()
        }
    }
    var minValue: Double {
        didSet {
            updateCell()
        }
    }
    var duration: SamplingDurationType {
        didSet {
            updateCell()
        }
    }
    
    // MARK: - Initializer
    init()
    {
        maxValue = 1.0
        minValue = 0
        duration = SamplingDurationType(milliSeconds: 100)    
    }

    deinit
    {
        cell = nil
    }

    // methods
    
    func updateCell()
    {
        cell?.maxValue = self.maxValue
        cell?.minValue = self.minValue
        cell?.duration = duration
    }
    
    func clearPlot()
    {
        logData = [[], [], []]
        cell?.graphView?.clearPlot()        
    }
    
    func drawRealTimeData(data: [Double])
    {
        // データを追加
        for (index, d) in data.enumerate() {
            logData[index].append(d)
        }
        
        // データが範囲を溢れたらクリア
        if logData[0].count > self.sampleCount {
            logData = [[], [], []]
        }
        
        // 描画
        self.cell?.graphView?.plotData(logData)
    }
    
    func startToReadLog(logid: UInt8)
    {
        self.logid = logid
        logData = [[], [], []]
        
        cell?.graphView?.clearPlot()
        cell?.progressBar?.progress = 0
        cell?.progressBar?.hidden   = false
    }
    
    func addReadLog(data:[Double]) {
        for (index, d) in data.enumerate() {
            logData[index].append(d)
        }
        
        cell?.graphView?.plotData(logData)
        cell?.progressBar?.progress = Float(logData[0].count) / Float(self.cell!.graphView!.sampleCount)
    }
    
    func stopReadingLog(fileName: String, duration: SamplingDurationType?)
    {
        // ファイルに保存
        let folderPath = NSSearchPathForDirectoriesInDomains(.DocumentDirectory,  .UserDomainMask, true).first! as NSString
        let filePath   = folderPath.stringByAppendingPathComponent("\(fileName)_\(self.logid).csv")
        saveToFile(filePath, data: logData, duration: duration!)
        
        // グラフの終了状態
        cell?.progressBar?.hidden    = true
    }
    
    private func saveToFile(filePath:String, data:[[Double]], duration: SamplingDurationType)
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
            if NSFileManager.defaultManager().fileExistsAtPath(filePath) {
                try NSFileManager.defaultManager().removeItemAtPath(filePath)
            }
            try content.writeToFile(filePath, atomically: true, encoding: NSUTF8StringEncoding)
        } catch {
            debugPrint("\(#function) fatal error in file save.")
        }
    }
    
    // Event
    @objc func iconButtonToutchUpInside(sender: UIButton)
    {}

    // SenStickSensorServiceDelegate
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
}
