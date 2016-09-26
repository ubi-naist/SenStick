//
//  SensorDataModel.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

protocol SensorDataModelDelegate :class {
    func didStopReadingLog(_ sender: SensorDataModel)
}

class SensorDataModel: SenStickSensorServiceDelegate {
    
    weak var delegate: SensorDataModelDelegate?
    
    let sampleCount: Int = 300
    
    var cell: SensorDataCellView? {
        willSet {
            cell?.iconButton?.removeTarget(self, action: #selector(iconButtonToutchUpInside), for: .touchUpInside)
        }
        didSet {
            cell?.iconButton?.addTarget(self, action: #selector(iconButtonToutchUpInside), for: .touchUpInside)
            if self.isLogReading {
                didUpdateMetaData(self)
            } else {
                didUpdateSetting(self)
            }
            cell?.graphView?.plotData(logData)
//debugPrint("\(#function)")
        }
    }
    var isLogReading: Bool = false
    
    var sensorName: String = "sensor"
    var csvHeader: String = "header"
    var csvEmptyData: String = ""
    
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
    convenience init(_ delegate: SensorDataModelDelegate?)
    {
        self.init()
        self.delegate = delegate
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
    
    func drawRealTimeData(_ data: [Double])
    {
        // データを追加
        for (index, d) in data.enumerated() {
            logData[index].append(d)
        }
        
        // データが範囲を溢れたらクリア
        if logData[0].count > self.sampleCount {
            logData = [[], [], []]
        }
        
        // 描画
        self.cell?.graphView?.plotData(logData)
    }
    
    func startToReadLog(_ logid: UInt8)
    {
        self.isLogReading = true
        
        self.logid = logid
        logData = [[], [], []]
        
        cell?.graphView?.clearPlot()
        cell?.progressBar?.progress = 0
        cell?.progressBar?.isHidden   = false
    }
    
    func addReadLog(_ data:[Double]) {
        for (index, d) in data.enumerated() {
            logData[index].append(d)
        }
        
        cell?.graphView?.plotData(logData)
        cell?.progressBar?.progress = Float(logData[0].count) / Float(self.cell!.graphView!.sampleCount)
    }
    
    // 指定したrowのデータをCSVテキストに変換。該当データがない場合は、CSVとしてデータが空の文字列を返す。
    func getCSVDataText(_ index:Int) -> String
    {
        let colomn  = logData.count
        var items = [String]()
        for c in 0..<colomn {
            if logData[c].count > index {
                items.append( "\((logData[c])[index])" )
            } else {
                break
            }
        }
        return items.joined(separator: ",\t")    
    }
    
    func saveToFile(_ filePath:String)
    {
        var content = ""
        let row     = logData[0].count
        
        // 浮動小数点型だと、インクリメントしていくと微妙な端数がでるので、ミリ秒単位で整数で扱う
        let samplingDuration = Int(duration.duration * 1000)
        var time :Int = 0
        for r in 0..<row {
            time    += samplingDuration
            content += "\(Double(time) / 1000),\t"
            content += self.getCSVDataText(r)
            content += "\n"
        }
        
        do {
            if FileManager.default.fileExists(atPath: filePath) {
                try FileManager.default.removeItem(atPath: filePath)
            }
            try content.write(toFile: filePath, atomically: true, encoding: String.Encoding.utf8)
        } catch {
            debugPrint("\(#function) fatal error in file save.")
        }
    }
    
    // Event
    @objc func iconButtonToutchUpInside(_ sender: UIButton)
    {}
    
    // SenStickSensorServiceDelegate
    func didUpdateSetting(_ sender:AnyObject)
    {}
    func didUpdateRealTimeData(_ sender: AnyObject)
    {}
    func didUpdateMetaData(_ sender: AnyObject)
    {}
    func didUpdateLogData(_ sender: AnyObject)
    {}
    func didFinishedLogData(_ sender: AnyObject)
    {
        // グラフの終了状態
        cell?.progressBar?.isHidden    = true
        if let dlgt = self.delegate {
            dlgt.didStopReadingLog(self)
        }
    }
}
