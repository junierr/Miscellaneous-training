package com.goinhn.kon.mapper.label;

import com.goinhn.kon.model.entity.label.BusinessBackgroundLabel;
import org.apache.ibatis.annotations.*;
import org.springframework.stereotype.Repository;

import javax.validation.constraints.NotNull;

/**
 * @author goinhn
 */
@Repository
public interface BusinessBackgroundLabelMapper {

    /**
     * 根据公司名返回企业背景标签序号数据
     *
     * @param businessBackgroundLabel 企业背景标签
     * @return
     */
    @Select("select * from tab_business_background_label where entname = #{entName}")
    @Results(
            id = "businessBackgroundLabelMap",
            value = {
                    @Result(id = true, column = "eid", property = "eid"),
                    @Result(column = "entname", property = "entName"),
                    @Result(column = "label", property = "label")
            }
    )
    BusinessBackgroundLabel selectAllByEntName(@NotNull BusinessBackgroundLabel businessBackgroundLabel);


    /**
     * 保存企业背景标签信息
     *
     * @param businessBackgroundLabel 企业背景标签
     * @return
     */
    @Insert("insert into " +
            "tab_business_background_label(eid, entname, label) " +
            "values(#{eid}, #{entName}, #{label})")
    int saveBusinessBackgroundLabel(@NotNull BusinessBackgroundLabel businessBackgroundLabel);


    /**
     * 根据id进行删除
     *
     * @param eid
     * @return
     */
    @Delete("delete from tab_business_background_label where eid = #{eid}")
    int deleteById(@NotNull Long eid);

}
