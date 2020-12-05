package com.goinhn.kon.model.entity.label;

import lombok.Builder;
import lombok.Data;

/**
 * 企业经营能力标签
 *
 * @author goinhn
 */
@Data
public class BusinessManagementAbilityLabel {

    /**
     * 序号
     */
    private Long eid;

    /**
     * 企业名称
     */
    private String entName;

    /**
     * 标签序号
     */
    private Integer label;
}
