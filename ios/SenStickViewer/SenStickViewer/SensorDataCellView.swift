//
//  SensorDataCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SensorDataCellView: UITableViewCell {
    @IBOutlet var titleTextLabel: UILabel?
    @IBOutlet var iconButton:     UIButton?
    @IBOutlet var maxTextLabel:   UILabel?
    @IBOutlet var minTextLabel:   UILabel?
    @IBOutlet var progressBar:    UIProgressView?
    @IBOutlet var graphView:      DataGraphView?
    
    // MARK: - Properties
    var maxValue: Double {
        didSet {
            maxTextLabel?.text = String(maxValue) + "/" + String(minValue)
            graphView?.maxValue = maxValue
        }
    }
    var minValue: Double {
        didSet {
            maxTextLabel?.text = String(maxValue) + "/" + String(minValue)
            graphView?.minValue = minValue
        }
    }
    var duration: SamplingDurationType {
        didSet {
            if duration.duration >= 1.0 {
                minTextLabel?.text = String(format: "%.1lf", duration.duration) + " s"
            } else {
                minTextLabel?.text = String(format: "%d", Int(duration.duration * 1000)) + " ms"
            }
        }
    }

    // MARK: - Initializer
    required init?(coder aDecoder: NSCoder)
    {
        maxValue = 1.0
        minValue = 0
        duration = SamplingDurationType(milliSeconds: 100)
        
        super.init(coder:aDecoder)
    }

}
