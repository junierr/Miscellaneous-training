package com.goinhn.kon.model.entity.analysis;

import lombok.Data;


/**
 * 企业司法风险分析
 *
 * @author goinhn
 */
@Data
public class JudicialRiskAnalysis {

    /**
     * eid：序号
     */
    private Long eid;

    /**
     * entName：公司名
     */
    private String entName;

    /**
     * lawSum: 司法次数
     */
    private Double lawSum;

    /**
     * defendant: 被诉方
     */
    private Double defendant;

    /**
     * enforceAmount: 执行标的
     */
    private Double enforceAmount;

    /**
     * isJusticeCredit: 是否列入失信黑名单
     */
    private Double isJusticeCredit;

}
