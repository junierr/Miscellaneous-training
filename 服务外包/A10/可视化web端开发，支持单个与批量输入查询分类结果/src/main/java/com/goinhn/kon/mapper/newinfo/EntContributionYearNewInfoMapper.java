package com.goinhn.kon.mapper.newinfo;

import com.goinhn.kon.model.entity.newinfo.EntContributionYearNewInfo;
import org.apache.ibatis.annotations.*;
import org.springframework.stereotype.Repository;

import javax.validation.constraints.NotNull;
import java.util.List;

/**
 * @author goinhn
 */
@Repository
public interface EntContributionYearNewInfoMapper {

    /**
     * 根据公司名称进行查询
     *
     * @param entContributionYearNewInfo 企业年报出资信息
     * @return
     */
    @Select("select * from tab_ent_contribution_year_new_info where entname = #{entName}")
    @Results(
            id = "entContributionYearNewInfoMap",
            value = {
                    @Result(id = true, column = "eid", property = "eid"),
                    @Result(column = "entname", property = "entName"),
                    @Result(column = "subconcurrency", property = "subconcurrency"),
                    @Result(column = "accondate", property = "acconDate"),
                    @Result(column = "subconform", property = "subconForm"),
                    @Result(column = "anchetype", property = "ancheType"),
                    @Result(column = "subcondate", property = "subconDate"),
                    @Result(column = "acconcurrency", property = "acconcurrency"),
                    @Result(column = "acconform", property = "acconForm"),
                    @Result(column = "liacconam", property = "liacconam"),
                    @Result(column = "lisubconam", property = "lisubconam")
            }
    )
    List<EntContributionYearNewInfo> selectAllByEntName(@NotNull EntContributionYearNewInfo entContributionYearNewInfo);


    /**
     * 保存企业年报出资信息
     *
     * @param entContributionYearNewInfo 企业年报出资信息
     * @return
     */
    @Insert("insert into " +
            "tab_ent_contribution_year_new_info(eid, entname, subconcurrency, accondate, subconform, anchetype, subcondate, " +
            "acconcurrency, acconform, liacconam, lisubconam) " +
            "values(#{eid}, #{entName}, #{subconcurrency}, #{acconDate}, #{subconForm}, #{ancheType}, #{subconDate}, " +
            "#{acconcurrency}, #{acconForm}, #{liacconam}, #{lisubconam})")
    @SelectKey(keyColumn = "eid", keyProperty = "eid",
            resultType = Long.class, before = false, statement = {"select last_insert_id()"})
    int saveEntContributionYearNewInfo(@NotNull EntContributionYearNewInfo entContributionYearNewInfo);
}
