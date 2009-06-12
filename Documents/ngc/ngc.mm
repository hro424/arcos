<map version="0.9.0_Beta_8">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node COLOR="#000000" CREATED="1210926383138" ID="ID_1688056223" MODIFIED="1211348881198" TEXT="A Framework for Recoverable Device Drivers">
<edge COLOR="#000000" WIDTH="8"/>
<font NAME="SansSerif" SIZE="20"/>
<hook NAME="accessories/plugins/AutomaticLayout.properties"/>
<node COLOR="#0033ff" CREATED="1210926441938" ID="ID_197299515" MODIFIED="1211267146006" POSITION="right" TEXT="State of drivers">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
<icon BUILTIN="messagebox_warning"/>
<node COLOR="#00b439" CREATED="1210926490926" ID="ID_754744576" MODIFIED="1211267146007" TEXT="Crash-only design">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211004898112" ID="ID_224356730" MODIFIED="1211267146008" TEXT="Control-data separation">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211005334177" ID="ID_289921840" MODIFIED="1211177985285" TEXT="Significant-non-significant data separation">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
<node COLOR="#990000" CREATED="1211005034359" ID="ID_1567796128" MODIFIED="1211267146008" TEXT="Strong isolation">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211004909433" ID="ID_1069886693" MODIFIED="1211267146009" TEXT="Timeout interactions">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211004920714" ID="ID_247641306" MODIFIED="1211267146009" TEXT="Resource leasing">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211005042235" ID="ID_624095082" MODIFIED="1211267146009" TEXT="Self-descriptive messages">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211005384099" ID="ID_1349267811" MODIFIED="1211267146009" TEXT="Types of state">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1210926890484" ID="ID_1888056438" MODIFIED="1211267146010" TEXT="Significant state">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1210926895606" ID="ID_447118079" MODIFIED="1211267146010" TEXT="Non-significant state">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211005421460" ID="ID_969944593" MODIFIED="1211267146010" TEXT="Method of separating state">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<icon BUILTIN="messagebox_warning"/>
<node COLOR="#990000" CREATED="1211178461507" ID="ID_573424280" MODIFIED="1211267146011" TEXT="Write a vanilla version">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211178476681" ID="ID_1369466774" MODIFIED="1211267146011" TEXT="Modify to the recoverable version">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
</node>
<node COLOR="#0033ff" CREATED="1210926501795" ID="ID_771771477" MODIFIED="1211267146011" POSITION="right" TEXT="Design and implementation">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
<node COLOR="#00b439" CREATED="1210926520484" ID="ID_116747504" MODIFIED="1211267146013" TEXT="ArcOS">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1210926906524" ID="ID_711889577" MODIFIED="1211267146013" TEXT="Microkernel">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1210926909717" ID="ID_124204401" MODIFIED="1211267146013" TEXT="Root task">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1210926913885" ID="ID_1720306872" MODIFIED="1211267146014" TEXT="P3 and memory mapping">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1210926524980" ID="ID_923913603" MODIFIED="1211267146014" TEXT="Driver class">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1210926980226" ID="ID_621019533" MODIFIED="1211267146014" TEXT="Requirements">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211266682418" ID="ID_310764198" MODIFIED="1211267146015" TEXT="Avoid not to add complexity">
<edge COLOR="#111111" WIDTH="thin"/>
<node COLOR="#111111" CREATED="1211266738884" ID="ID_353500513" MODIFIED="1211267146015" TEXT="SQL is more complex">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211266751516" ID="ID_751832494" MODIFIED="1211267146015" TEXT="Device driver developers should accept this degree of complexity.">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
</node>
<node COLOR="#990000" CREATED="1210926988986" ID="ID_139998133" MODIFIED="1211267146015" TEXT="C++ base class">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1210926984210" ID="ID_162243040" MODIFIED="1211267146015" TEXT="Usage">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1210926530172" ID="ID_684596235" MODIFIED="1211267146016" TEXT="Persistent memory">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1210926923069" ID="ID_524209129" MODIFIED="1211267146016" TEXT="Requirements">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211266617937" ID="ID_1315520921" MODIFIED="1211267146017" TEXT="Avoid overflow">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211266642714" ID="ID_1667608464" MODIFIED="1211267146017" TEXT="Atomic access">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211266666554" ID="ID_18834523" MODIFIED="1211267146017" TEXT="Preserve data across restart">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
<node COLOR="#990000" CREATED="1210926943929" ID="ID_169995744" MODIFIED="1211267146017" TEXT="C macro">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1210926951001" ID="ID_1798270689" MODIFIED="1211267146017" TEXT="Memory layout">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1210926960553" ID="ID_1453607755" MODIFIED="1211267146018" TEXT="Usage">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
</node>
<node COLOR="#0033ff" CREATED="1210926534556" ID="ID_981521645" MODIFIED="1211267146018" POSITION="right" TEXT="Evaluation">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
<node COLOR="#00b439" CREATED="1210926537436" ID="ID_1335225093" MODIFIED="1211267146019" TEXT="Case studies">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211173324768" ID="ID_1269084092" MODIFIED="1211267146020" TEXT="Components">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1210926540820" ID="ID_1437550533" MODIFIED="1211177985296" TEXT="Parallel port">
<edge COLOR="#111111" WIDTH="thin"/>
<font NAME="SansSerif" SIZE="12"/>
</node>
<node COLOR="#111111" CREATED="1210926543716" ID="ID_824021977" MODIFIED="1211177985296" TEXT="VESA VGA">
<edge COLOR="#111111" WIDTH="thin"/>
<font NAME="SansSerif" SIZE="12"/>
</node>
<node COLOR="#111111" CREATED="1210926548308" ID="ID_1362711398" MODIFIED="1211177985296" TEXT="Audio">
<edge COLOR="#111111" WIDTH="thin"/>
<font NAME="SansSerif" SIZE="12"/>
</node>
<node COLOR="#111111" CREATED="1210926554876" ID="ID_1602369488" MODIFIED="1211177985296" TEXT="Disk">
<edge COLOR="#111111" WIDTH="thin"/>
<font NAME="SansSerif" SIZE="12"/>
</node>
<node COLOR="#111111" CREATED="1211000019235" ID="ID_1139878585" MODIFIED="1211177985297" TEXT="File system">
<edge COLOR="#111111" WIDTH="thin"/>
<font NAME="SansSerif" SIZE="12"/>
</node>
</node>
<node COLOR="#990000" CREATED="1211173342088" ID="ID_1195991189" MODIFIED="1211267146021" TEXT="Comparison">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211173346888" ID="ID_1424126632" MODIFIED="1211177985297" TEXT="Under the framework">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211173352264" ID="ID_1507081077" MODIFIED="1211177985297" TEXT="Vanilla">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
</node>
<node COLOR="#00b439" CREATED="1210926559460" ID="ID_134626418" MODIFIED="1211267146021" TEXT="Performance">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1210927058563" ID="ID_518537612" MODIFIED="1211267146022" TEXT="Time of recovery">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211178636684" ID="ID_1612988005" MODIFIED="1211252428653" TEXT="Scale with the size of a driver">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
</node>
<node COLOR="#00b439" CREATED="1211268421199" ID="ID_389442111" MODIFIED="1211268426861" TEXT="Proof">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211268427458" ID="ID_224306124" MODIFIED="1211268461306" TEXT="Tradeoff between reliability and development cost">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
</node>
<node COLOR="#0033ff" CREATED="1210926564972" ID="ID_462082879" MODIFIED="1211267146023" POSITION="right" TEXT="Discussion">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
</node>
<node COLOR="#0033ff" CREATED="1211252405527" ID="ID_615124509" MODIFIED="1211267146023" POSITION="left" TEXT="Context">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
<node COLOR="#00b439" CREATED="1211252408212" ID="ID_1202376667" MODIFIED="1211267146024" TEXT="Multiserver for embedded systems">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211252480995" ID="ID_1487543622" MODIFIED="1211267146024" TEXT="Mobile phones">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211252487795" ID="ID_1348126446" MODIFIED="1211267146024" TEXT="Car">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211252491107" ID="ID_974678629" MODIFIED="1211267146025" TEXT="Consumer electronics">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211252433138" ID="ID_1806994987" MODIFIED="1211267146025" TEXT="Reliable device drivers in general">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
<node COLOR="#00b439" CREATED="1211252595381" ID="ID_484418433" MODIFIED="1211267146025" TEXT="Cyber-Physical Systems">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
</node>
<node COLOR="#0033ff" CREATED="1210926421859" ID="ID_234299034" MODIFIED="1211267146025" POSITION="left" TEXT="Introduction">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
<node COLOR="#00b439" CREATED="1211184005368" ID="ID_426591336" MODIFIED="1211267146030" TEXT="Meta-background">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211184010289" ID="ID_1739086136" MODIFIED="1211267146030" TEXT="Bugs never disappear.">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211001670412" ID="ID_586571544" MODIFIED="1211267146031" TEXT="Background">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211267924759" ID="ID_1873203539" MODIFIED="1211267940627" TEXT="Operating systems play a central role of any computer systems.">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211267456379" ID="ID_608507843" MODIFIED="1211267820776" TEXT="The major cause of operating system failure is device drivers.">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211267582367" ID="ID_1170483632" MODIFIED="1211267618139" TEXT="[Swift et al. 2006]">
<edge COLOR="#111111"/>
</node>
<node COLOR="#111111" CREATED="1211267597919" ID="ID_1084873844" MODIFIED="1211267610403" TEXT="[Chou et al. 2001]">
<edge COLOR="#111111"/>
</node>
</node>
<node COLOR="#990000" CREATED="1210927175551" FOLDED="true" ID="ID_1136620671" MODIFIED="1211267836609" TEXT="Device drivers are prone to errors.">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211163741689" ID="ID_1163687670" MODIFIED="1211177985303" TEXT="Frequently-changing parts in an operating system">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211163820919" ID="ID_406884376" MODIFIED="1211177985303" TEXT="Not always comprehensive specifications">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211164493470" ID="ID_24189849" MODIFIED="1211177985303" TEXT="Directly communicates with hardware">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
<node COLOR="#990000" CREATED="1211173284111" ID="ID_1416980463" MODIFIED="1211267146033" TEXT="Need a technology for reliable device drivers">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211010555386" ID="ID_1432744991" MODIFIED="1211267146033" TEXT="Brief history of device drivers and OS extensions and dependability technologies">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211163271867" ID="ID_1721075991" MODIFIED="1211177985304" TEXT="Bluescreen of death">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211163293931" ID="ID_1307775421" MODIFIED="1211177985304" TEXT="Isolation">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211163377045" ID="ID_1494164571" MODIFIED="1211177985304" TEXT="Domain specific language">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211163400310" ID="ID_163890770" MODIFIED="1211177985305" TEXT="Reincurnation">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211163392102" ID="ID_1787248324" MODIFIED="1211177985305" TEXT="Shadowing">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
<node COLOR="#990000" CREATED="1211164662081" ID="ID_880547212" MODIFIED="1211267146034" TEXT="Existing technologies for dependable device drivers">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211164709642" ID="ID_840362720" MODIFIED="1211177985305" TEXT="Removing bugs">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211164714259" ID="ID_1833195201" MODIFIED="1211177985306" TEXT="Reusing proprietary device drivers">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211164722667" ID="ID_1733031161" MODIFIED="1211177985306" TEXT="Transparent failure recovery">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
</node>
<node COLOR="#00b439" CREATED="1211173026808" ID="ID_1779234882" MODIFIED="1211267146034" TEXT="Problem">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211173204661" ID="ID_1450936231" MODIFIED="1211267146035" TEXT="Recovery procedures are centralized in the underlying system.">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211178229819" ID="ID_681936821" MODIFIED="1211267146035" TEXT="No uniform way to recover device drivers?">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211267125640" ID="ID_365214194" MODIFIED="1211267861493" TEXT="The way of developing device drivers should be changed.">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211267177582" ID="ID_853412140" MODIFIED="1211267861493" TEXT="Failure recovery should be taken into account (Recovery-oriented computing)">
<edge COLOR="#111111" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="12"/>
<node COLOR="#111111" CREATED="1211267230895" ID="ID_711084774" MODIFIED="1211267233763" TEXT="How?">
<edge COLOR="#111111"/>
<node COLOR="#111111" CREATED="1211267243991" ID="ID_1703143419" MODIFIED="1211267253891" TEXT="Crash-only software">
<edge COLOR="#111111"/>
</node>
</node>
</node>
</node>
</node>
<node COLOR="#00b439" CREATED="1211001675499" ID="ID_264811951" MODIFIED="1211267146035" TEXT="Purpose">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211163857007" ID="ID_347055156" MODIFIED="1211267146036" TEXT="To develop a systems that tolerate to the device driver errors">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211009381068" ID="ID_1759352781" MODIFIED="1211267146036" TEXT="To let programmers consider the recovery of device drivers">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211001678147" ID="ID_67717152" MODIFIED="1211267146036" TEXT="Contribution">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211164617897" ID="ID_763729813" MODIFIED="1211267146037" TEXT="To let programmers in the recovery process">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
<node COLOR="#111111" CREATED="1211165204093" ID="ID_30708431" MODIFIED="1211177985309" TEXT="Framework">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
<node COLOR="#111111" CREATED="1211165215037" ID="ID_1179332483" MODIFIED="1211177985309" TEXT="Guideline">
<edge COLOR="#111111" WIDTH="thin"/>
</node>
</node>
</node>
</node>
<node COLOR="#0033ff" CREATED="1210926430514" ID="ID_1505370681" MODIFIED="1211267146037" POSITION="left" TEXT="Related work">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
<node COLOR="#00b439" CREATED="1211001751949" ID="ID_1618607390" MODIFIED="1211267146039" TEXT="Restart recovery in general">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
<node COLOR="#00b439" CREATED="1210926609661" ID="ID_1258022384" MODIFIED="1211267146040" TEXT="Rio/Vista">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1210926600213" ID="ID_736677828" MODIFIED="1211267146040" TEXT="Consistent recovery">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211000054254" ID="ID_1802193052" MODIFIED="1211267146040" TEXT="Transaction">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1210926623454" ID="ID_532041351" MODIFIED="1211267146041" TEXT="Nooks">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211000149216" ID="ID_1608259069" MODIFIED="1211267146041" TEXT="Shadow driver">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1210926626902" ID="ID_502691122" MODIFIED="1211267146041" TEXT="Chorus">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211000144161" ID="ID_763905772" MODIFIED="1211267146041" TEXT="Hot restart">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1210926629374" ID="ID_1332664724" MODIFIED="1211267146042" TEXT="Choices">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211000122056" ID="ID_282227279" MODIFIED="1211267146042" TEXT="CPU exception in C++ exception">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1210926634454" ID="ID_1191385598" MODIFIED="1211267146042" TEXT="MINIX3">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211000113056" ID="ID_444193712" MODIFIED="1211267146043" TEXT="Reincurnation server">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211000066559" ID="ID_308677968" MODIFIED="1211267146043" TEXT="SPIN">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<node COLOR="#990000" CREATED="1211000068999" ID="ID_1862568886" MODIFIED="1211267146043" TEXT="Transactional memory">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211000080191" ID="ID_382847409" MODIFIED="1211267146044" TEXT="Secure modules">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211000097559" ID="ID_1551571345" MODIFIED="1211267146044" TEXT="VINO">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
<node COLOR="#00b439" CREATED="1211000353110" ID="ID_983857916" MODIFIED="1211267146044" TEXT="Driver development in UNIX">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<icon BUILTIN="messagebox_warning"/>
<node COLOR="#990000" CREATED="1211000362582" ID="ID_1993021136" MODIFIED="1211267146045" TEXT="Linux">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211000364302" ID="ID_417757526" MODIFIED="1211267146045" TEXT="BSD">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
<node COLOR="#990000" CREATED="1211000369406" ID="ID_101050890" MODIFIED="1211267146045" TEXT="OSX IOKit">
<edge COLOR="#990000" STYLE="bezier" WIDTH="1"/>
<font NAME="SansSerif" SIZE="14"/>
</node>
</node>
<node COLOR="#00b439" CREATED="1211000380222" ID="ID_549586248" MODIFIED="1211267146045" TEXT="Driver development in Windows">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
<icon BUILTIN="messagebox_warning"/>
</node>
<node COLOR="#00b439" CREATED="1211267291176" ID="ID_64318470" MODIFIED="1211267295237" TEXT="Recovery-oriented computing">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
</node>
<node COLOR="#0033ff" CREATED="1210926568164" ID="ID_192853203" MODIFIED="1211267146046" POSITION="right" TEXT="Conclusion">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
</node>
<node COLOR="#0033ff" CREATED="1211177888483" ID="ID_1372118417" MODIFIED="1211267146046" POSITION="right" TEXT="Grocessary">
<edge COLOR="#0033ff" STYLE="bezier" WIDTH="4"/>
<font NAME="SansSerif" SIZE="18"/>
<node COLOR="#00b439" CREATED="1211177987003" ID="ID_1063829630" MODIFIED="1211267146047" TEXT="Fault">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
<node COLOR="#00b439" CREATED="1211177990869" ID="ID_735197511" MODIFIED="1211267146047" TEXT="Error">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
<node COLOR="#00b439" CREATED="1211177992725" ID="ID_951948018" MODIFIED="1211267146047" TEXT="Failure">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
<node COLOR="#00b439" CREATED="1211178002086" ID="ID_683152801" MODIFIED="1211267146047" TEXT="Reliability">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
<node COLOR="#00b439" CREATED="1211178009214" ID="ID_1891849441" MODIFIED="1211267146048" TEXT="Dependability">
<edge COLOR="#00b439" STYLE="bezier" WIDTH="2"/>
<font NAME="SansSerif" SIZE="16"/>
</node>
</node>
</node>
</map>
