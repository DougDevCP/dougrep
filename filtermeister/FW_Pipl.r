#pragma warning(push)
#pragma warning(disable:4005)
//#include "FW_FML.cpp"
#include "../../_FilterCodeFile.h"

#include "PIDefines.h"
#define Rez
#include "PIGeneral.h"
#include "PIUtilities.r"
//#include "WinDialogUtils.r"
#include "PIActions.h"


// Default names.
#ifndef plugInName
#define plugInName "My Filter"
#endif
#ifndef plugInNameEllipsis
#define plugInNameEllipsis "..."
#endif
#ifndef plugInVendorName
#define plugInVendorName "My Category"
#endif
#ifndef plugInSupportedModes
#define plugInSupportedModes doesSupportBitmap, doesSupportGrayScale, doesSupportIndexedColor, doesSupportRGBColor, doesSupportCMYKColor, doesSupportHSLColor, doesSupportHSBColor, doesSupportMultichannel, doesSupportDuotone, doesSupportLABColor
#endif
#ifndef plugInEnableInfo
#define plugInEnableInfo "in (PSHOP_ImageMode, RGBMode, GrayScaleMode, CMYKMode, HSLMode, HSBMode, MultichannelMode, DuotoneMode, LabMode, RGB48Mode, Gray16Mode)"
#endif
#ifndef plugInUniqueID
#define plugInUniqueID plugInVendorName plugInName
#endif
#ifndef plugInSuiteID
#define plugInSuiteID		plugInName
#endif
#ifndef plugInClassID
#define plugInClassID		plugInSuiteID
#endif
#ifndef plugInEventID
#define plugInEventID		plugInClassID
#endif



resource 'PiPL' ( 16000, plugInUniqueID " PiPL", purgeable )
{
	{
		Kind { Filter },
		Name { plugInName plugInNameEllipsis },
		Category { plugInVendorName },
		Version { (latestFilterVersion << 16 ) | latestFilterSubVersion },

		
		#if defined(_WIN64)
	        CodeWin64X86 { "ENTRYPOINT" },
		#else
    		CodeWin32X86 { "ENTRYPOINT" },
		#endif
		
		
		//Convertpipl.exe doesn not support 16bit modes, so we need to set 0xffff in the .rc file ourselves
		SupportedModes
		{
			plugInSupportedModes
		}, 
		
	// #if defined(SCRIPTABLE)
		HasTerminology
		{
			plugInClassID,
			plugInEventID,
			16000,
			plugInUniqueID
		},
		
		FilterLayerSupport {doesSupportFilterLayers},
	// #endif
		
		EnableInfo { plugInEnableInfo }, //"in (PSHOP_ImageMode, RGBMode, RGB48Mode) || PSHOP_ImageDepth == 16"

     #if defined(BIGDOCUMENT)
		PlugInMaxSize { 2000000, 2000000 },
	 #endif

		FilterCaseInfo
		{
			{
				/* Flat data, no selection */
				inStraightData, outStraightData,
				doNotWriteOutsideSelection,
				doesNotFilterLayerMasks, doesNotWorkWithBlankData,
				doNotCopySourceToDestination,
					
				/* Flat data with selection */
				inStraightData, outStraightData,
				doNotWriteOutsideSelection,
				doesNotFilterLayerMasks, doesNotWorkWithBlankData,
				doNotCopySourceToDestination,
				
				/* Floating selection */
				inBackgroundZap, outStraightData,
				doNotWriteOutsideSelection,
				doesNotFilterLayerMasks, doesNotWorkWithBlankData,
				doNotCopySourceToDestination,
					
				/* Editable transparency, no selection */
				inBackgroundZap, outStraightData,
				doNotWriteOutsideSelection,
				doesNotFilterLayerMasks, worksWithBlankData,
				doNotCopySourceToDestination,
					
				/* Editable transparency, with selection */
				inBackgroundZap, outStraightData,
				doNotWriteOutsideSelection,
				doesNotFilterLayerMasks, worksWithBlankData,
				doNotCopySourceToDestination,
					
				/* Preserved transparency, no selection */
				inBackgroundZap, outStraightData,
				doNotWriteOutsideSelection,
				doesNotFilterLayerMasks, doesNotWorkWithBlankData,
				doNotCopySourceToDestination,
					
				/* Preserved transparency, with selection */
				inBackgroundZap, outStraightData,
				doNotWriteOutsideSelection,
				doesNotFilterLayerMasks, doesNotWorkWithBlankData,
				doNotCopySourceToDestination
			}
		}	
	}
};



// #if defined(SCRIPTABLE)
resource 'aete' (16000, "Dissolve dictionary", purgeable)
{
	1, 0, english, roman,									/* aete version and language specifiers */
	{
		plugInVendorName,											/* vendor suite name */
		plugInDescription,							/* optional description */
		plugInSuiteID,										/* suite ID */
		1,													/* suite code, must be 1 */
		1,													/* suite level, must be 1 */
		{													/* structure for filters */
			plugInUniqueID,										/* unique filter name */
			plugInAETEComment,								/* optional description */
			plugInClassID,									/* class ID, must be unique or Suite ID */
			plugInEventID,									/* event ID, must be unique to class ID */
			
			NO_REPLY,										/* never a reply */
			IMAGE_DIRECT_PARAMETER,							/* direct parameter, used by Photoshop */
			{												/* parameters here, if any */
			
				"ctl000",'0000',typeInteger,"",flagsSingleParameter,
				"ctl001",'0001',typeInteger,"",flagsSingleParameter,
				"ctl002",'0002',typeInteger,"",flagsSingleParameter,
				"ctl003",'0003',typeInteger,"",flagsSingleParameter,
				"ctl004",'0004',typeInteger,"",flagsSingleParameter,
				"ctl005",'0005',typeInteger,"",flagsSingleParameter,
				"ctl006",'0006',typeInteger,"",flagsSingleParameter,
				"ctl007",'0007',typeInteger,"",flagsSingleParameter,
				"ctl008",'0008',typeInteger,"",flagsSingleParameter,
				"ctl009",'0009',typeInteger,"",flagsSingleParameter,
					
				"ctl010",'0010',typeInteger,"",flagsSingleParameter,
				"ctl011",'0011',typeInteger,"",flagsSingleParameter,
				"ctl012",'0012',typeInteger,"",flagsSingleParameter,
				"ctl013",'0013',typeInteger,"",flagsSingleParameter,
				"ctl014",'0014',typeInteger,"",flagsSingleParameter,
				"ctl015",'0015',typeInteger,"",flagsSingleParameter,
				"ctl016",'0016',typeInteger,"",flagsSingleParameter,
				"ctl017",'0017',typeInteger,"",flagsSingleParameter,
				"ctl018",'0018',typeInteger,"",flagsSingleParameter,
				"ctl019",'0019',typeInteger,"",flagsSingleParameter,
					
				"ctl020",'0020',typeInteger,"",flagsSingleParameter,
				"ctl021",'0021',typeInteger,"",flagsSingleParameter,
				"ctl022",'0022',typeInteger,"",flagsSingleParameter,
				"ctl023",'0023',typeInteger,"",flagsSingleParameter,
				"ctl024",'0024',typeInteger,"",flagsSingleParameter,
				"ctl025",'0025',typeInteger,"",flagsSingleParameter,
				"ctl026",'0026',typeInteger,"",flagsSingleParameter,
				"ctl027",'0027',typeInteger,"",flagsSingleParameter,
				"ctl028",'0028',typeInteger,"",flagsSingleParameter,
				"ctl029",'0029',typeInteger,"",flagsSingleParameter,
					
				"ctl030",'0030',typeInteger,"",flagsSingleParameter,
				"ctl031",'0031',typeInteger,"",flagsSingleParameter,
				"ctl032",'0032',typeInteger,"",flagsSingleParameter,
				"ctl033",'0033',typeInteger,"",flagsSingleParameter,
				"ctl034",'0034',typeInteger,"",flagsSingleParameter,
				"ctl035",'0035',typeInteger,"",flagsSingleParameter,
				"ctl036",'0036',typeInteger,"",flagsSingleParameter,
				"ctl037",'0037',typeInteger,"",flagsSingleParameter,
				"ctl038",'0038',typeInteger,"",flagsSingleParameter,
				"ctl039",'0039',typeInteger,"",flagsSingleParameter,
					
				"ctl040",'0040',typeInteger,"",flagsSingleParameter,
				"ctl041",'0041',typeInteger,"",flagsSingleParameter,
				"ctl042",'0042',typeInteger,"",flagsSingleParameter,
				"ctl043",'0043',typeInteger,"",flagsSingleParameter,
				"ctl044",'0044',typeInteger,"",flagsSingleParameter,
				"ctl045",'0045',typeInteger,"",flagsSingleParameter,
				"ctl046",'0046',typeInteger,"",flagsSingleParameter,
				"ctl047",'0047',typeInteger,"",flagsSingleParameter,
				"ctl048",'0048',typeInteger,"",flagsSingleParameter,
				"ctl049",'0049',typeInteger,"",flagsSingleParameter,
					
				"ctl050",'0050',typeInteger,"",flagsSingleParameter,
				"ctl051",'0051',typeInteger,"",flagsSingleParameter,
				"ctl052",'0052',typeInteger,"",flagsSingleParameter,
				"ctl053",'0053',typeInteger,"",flagsSingleParameter,
				"ctl054",'0054',typeInteger,"",flagsSingleParameter,
				"ctl055",'0055',typeInteger,"",flagsSingleParameter,
				"ctl056",'0056',typeInteger,"",flagsSingleParameter,
				"ctl057",'0057',typeInteger,"",flagsSingleParameter,
				"ctl058",'0058',typeInteger,"",flagsSingleParameter,
				"ctl059",'0059',typeInteger,"",flagsSingleParameter,
					
				"ctl060",'0060',typeInteger,"",flagsSingleParameter,
				"ctl061",'0061',typeInteger,"",flagsSingleParameter,
				"ctl062",'0062',typeInteger,"",flagsSingleParameter,
				"ctl063",'0063',typeInteger,"",flagsSingleParameter,
				"ctl064",'0064',typeInteger,"",flagsSingleParameter,
				"ctl065",'0065',typeInteger,"",flagsSingleParameter,
				"ctl066",'0066',typeInteger,"",flagsSingleParameter,
				"ctl067",'0067',typeInteger,"",flagsSingleParameter,
				"ctl068",'0068',typeInteger,"",flagsSingleParameter,
				"ctl069",'0069',typeInteger,"",flagsSingleParameter,
					
				"ctl070",'0070',typeInteger,"",flagsSingleParameter,
				"ctl071",'0071',typeInteger,"",flagsSingleParameter,
				"ctl072",'0072',typeInteger,"",flagsSingleParameter,
				"ctl073",'0073',typeInteger,"",flagsSingleParameter,
				"ctl074",'0074',typeInteger,"",flagsSingleParameter,
				"ctl075",'0075',typeInteger,"",flagsSingleParameter,
				"ctl076",'0076',typeInteger,"",flagsSingleParameter,
				"ctl077",'0077',typeInteger,"",flagsSingleParameter,
				"ctl078",'0078',typeInteger,"",flagsSingleParameter,
				"ctl079",'0079',typeInteger,"",flagsSingleParameter,
					
				"ctl080",'0080',typeInteger,"",flagsSingleParameter,
				"ctl081",'0081',typeInteger,"",flagsSingleParameter,
				"ctl082",'0082',typeInteger,"",flagsSingleParameter,
				"ctl083",'0083',typeInteger,"",flagsSingleParameter,
				"ctl084",'0084',typeInteger,"",flagsSingleParameter,
				"ctl085",'0085',typeInteger,"",flagsSingleParameter,
				"ctl086",'0086',typeInteger,"",flagsSingleParameter,
				"ctl087",'0087',typeInteger,"",flagsSingleParameter,
				"ctl088",'0088',typeInteger,"",flagsSingleParameter,
				"ctl089",'0089',typeInteger,"",flagsSingleParameter,
					
				"ctl090",'0090',typeInteger,"",flagsSingleParameter,
				"ctl091",'0091',typeInteger,"",flagsSingleParameter,
				"ctl092",'0092',typeInteger,"",flagsSingleParameter,
				"ctl093",'0093',typeInteger,"",flagsSingleParameter,
				"ctl094",'0094',typeInteger,"",flagsSingleParameter,
				"ctl095",'0095',typeInteger,"",flagsSingleParameter,
				"ctl096",'0096',typeInteger,"",flagsSingleParameter,
				"ctl097",'0097',typeInteger,"",flagsSingleParameter,
				"ctl098",'0098',typeInteger,"",flagsSingleParameter,
				"ctl099",'0099',typeInteger,"",flagsSingleParameter,
					
					
					
				"ctl100",'0100',typeInteger,"",flagsSingleParameter,
				"ctl101",'0101',typeInteger,"",flagsSingleParameter,
				"ctl102",'0102',typeInteger,"",flagsSingleParameter,
				"ctl103",'0103',typeInteger,"",flagsSingleParameter,
				"ctl104",'0104',typeInteger,"",flagsSingleParameter,
				"ctl105",'0105',typeInteger,"",flagsSingleParameter,
				"ctl106",'0106',typeInteger,"",flagsSingleParameter,
				"ctl107",'0107',typeInteger,"",flagsSingleParameter,
				"ctl108",'0108',typeInteger,"",flagsSingleParameter,
				"ctl109",'0109',typeInteger,"",flagsSingleParameter,
					              
				"ctl110",'0110',typeInteger,"",flagsSingleParameter,
				"ctl111",'0111',typeInteger,"",flagsSingleParameter,
				"ctl112",'0112',typeInteger,"",flagsSingleParameter,
				"ctl113",'0113',typeInteger,"",flagsSingleParameter,
				"ctl114",'0114',typeInteger,"",flagsSingleParameter,
				"ctl115",'0115',typeInteger,"",flagsSingleParameter,
				"ctl116",'0116',typeInteger,"",flagsSingleParameter,
				"ctl117",'0117',typeInteger,"",flagsSingleParameter,
				"ctl118",'0118',typeInteger,"",flagsSingleParameter,
				"ctl119",'0119',typeInteger,"",flagsSingleParameter,
					              
				"ctl120",'0120',typeInteger,"",flagsSingleParameter,
				"ctl121",'0121',typeInteger,"",flagsSingleParameter,
				"ctl122",'0122',typeInteger,"",flagsSingleParameter,
				"ctl123",'0123',typeInteger,"",flagsSingleParameter,
				"ctl124",'0124',typeInteger,"",flagsSingleParameter,
				"ctl125",'0125',typeInteger,"",flagsSingleParameter,
				"ctl126",'0126',typeInteger,"",flagsSingleParameter,
				"ctl127",'0127',typeInteger,"",flagsSingleParameter,
				"ctl128",'0128',typeInteger,"",flagsSingleParameter,
				"ctl129",'0129',typeInteger,"",flagsSingleParameter,
					              
				"ctl130",'0130',typeInteger,"",flagsSingleParameter,
				"ctl131",'0131',typeInteger,"",flagsSingleParameter,
				"ctl132",'0132',typeInteger,"",flagsSingleParameter,
				"ctl133",'0133',typeInteger,"",flagsSingleParameter,
				"ctl134",'0134',typeInteger,"",flagsSingleParameter,
				"ctl135",'0135',typeInteger,"",flagsSingleParameter,
				"ctl136",'0136',typeInteger,"",flagsSingleParameter,
				"ctl137",'0137',typeInteger,"",flagsSingleParameter,
				"ctl138",'0138',typeInteger,"",flagsSingleParameter,
				"ctl139",'0139',typeInteger,"",flagsSingleParameter,
					              
				"ctl140",'0140',typeInteger,"",flagsSingleParameter,
				"ctl141",'0141',typeInteger,"",flagsSingleParameter,
				"ctl142",'0142',typeInteger,"",flagsSingleParameter,
				"ctl143",'0143',typeInteger,"",flagsSingleParameter,
				"ctl144",'0144',typeInteger,"",flagsSingleParameter,
				"ctl145",'0145',typeInteger,"",flagsSingleParameter,
				"ctl146",'0146',typeInteger,"",flagsSingleParameter,
				"ctl147",'0147',typeInteger,"",flagsSingleParameter,
				"ctl148",'0148',typeInteger,"",flagsSingleParameter,
				"ctl149",'0149',typeInteger,"",flagsSingleParameter,
					              
				"ctl150",'0150',typeInteger,"",flagsSingleParameter,
				"ctl151",'0151',typeInteger,"",flagsSingleParameter,
				"ctl152",'0152',typeInteger,"",flagsSingleParameter,
				"ctl153",'0153',typeInteger,"",flagsSingleParameter,
				"ctl154",'0154',typeInteger,"",flagsSingleParameter,
				"ctl155",'0155',typeInteger,"",flagsSingleParameter,
				"ctl156",'0156',typeInteger,"",flagsSingleParameter,
				"ctl157",'0157',typeInteger,"",flagsSingleParameter,
				"ctl158",'0158',typeInteger,"",flagsSingleParameter,
				"ctl159",'0159',typeInteger,"",flagsSingleParameter,
					              
				"ctl160",'0160',typeInteger,"",flagsSingleParameter,
				"ctl161",'0161',typeInteger,"",flagsSingleParameter,
				"ctl162",'0162',typeInteger,"",flagsSingleParameter,
				"ctl163",'0163',typeInteger,"",flagsSingleParameter,
				"ctl164",'0164',typeInteger,"",flagsSingleParameter,
				"ctl165",'0165',typeInteger,"",flagsSingleParameter,
				"ctl166",'0166',typeInteger,"",flagsSingleParameter,
				"ctl167",'0167',typeInteger,"",flagsSingleParameter,
				"ctl168",'0168',typeInteger,"",flagsSingleParameter,
				"ctl169",'0169',typeInteger,"",flagsSingleParameter,
					              
				"ctl170",'0170',typeInteger,"",flagsSingleParameter,
				"ctl171",'0171',typeInteger,"",flagsSingleParameter,
				"ctl172",'0172',typeInteger,"",flagsSingleParameter,
				"ctl173",'0173',typeInteger,"",flagsSingleParameter,
				"ctl174",'0174',typeInteger,"",flagsSingleParameter,
				"ctl175",'0175',typeInteger,"",flagsSingleParameter,
				"ctl176",'0176',typeInteger,"",flagsSingleParameter,
				"ctl177",'0177',typeInteger,"",flagsSingleParameter,
				"ctl178",'0178',typeInteger,"",flagsSingleParameter,
				"ctl179",'0179',typeInteger,"",flagsSingleParameter,
					              
				"ctl180",'0180',typeInteger,"",flagsSingleParameter,
				"ctl181",'0181',typeInteger,"",flagsSingleParameter,
				"ctl182",'0182',typeInteger,"",flagsSingleParameter,
				"ctl183",'0183',typeInteger,"",flagsSingleParameter,
				"ctl184",'0184',typeInteger,"",flagsSingleParameter,
				"ctl185",'0185',typeInteger,"",flagsSingleParameter,
				"ctl186",'0186',typeInteger,"",flagsSingleParameter,
				"ctl187",'0187',typeInteger,"",flagsSingleParameter,
				"ctl188",'0188',typeInteger,"",flagsSingleParameter,
				"ctl189",'0189',typeInteger,"",flagsSingleParameter,
					              
				"ctl190",'0190',typeInteger,"",flagsSingleParameter,
				"ctl191",'0191',typeInteger,"",flagsSingleParameter,
				"ctl192",'0192',typeInteger,"",flagsSingleParameter,
				"ctl193",'0193',typeInteger,"",flagsSingleParameter,
				"ctl194",'0194',typeInteger,"",flagsSingleParameter,
				"ctl195",'0195',typeInteger,"",flagsSingleParameter,
				"ctl196",'0196',typeInteger,"",flagsSingleParameter,
				"ctl197",'0197',typeInteger,"",flagsSingleParameter,
				"ctl198",'0198',typeInteger,"",flagsSingleParameter,
				"ctl199",'0199',typeInteger,"",flagsSingleParameter,
					
					
					
				"ctl200",'0200',typeInteger,"",flagsSingleParameter,
				"ctl201",'0201',typeInteger,"",flagsSingleParameter,
				"ctl202",'0202',typeInteger,"",flagsSingleParameter,
				"ctl203",'0203',typeInteger,"",flagsSingleParameter,
				"ctl204",'0204',typeInteger,"",flagsSingleParameter,
				"ctl205",'0205',typeInteger,"",flagsSingleParameter,
				"ctl206",'0206',typeInteger,"",flagsSingleParameter,
				"ctl207",'0207',typeInteger,"",flagsSingleParameter,
				"ctl208",'0208',typeInteger,"",flagsSingleParameter,
				"ctl209",'0209',typeInteger,"",flagsSingleParameter,
					              
				"ctl210",'0210',typeInteger,"",flagsSingleParameter,
				"ctl211",'0211',typeInteger,"",flagsSingleParameter,
				"ctl212",'0212',typeInteger,"",flagsSingleParameter,
				"ctl213",'0213',typeInteger,"",flagsSingleParameter,
				"ctl214",'0214',typeInteger,"",flagsSingleParameter,
				"ctl215",'0215',typeInteger,"",flagsSingleParameter,
				"ctl216",'0216',typeInteger,"",flagsSingleParameter,
				"ctl217",'0217',typeInteger,"",flagsSingleParameter,
				"ctl218",'0218',typeInteger,"",flagsSingleParameter,
				"ctl219",'0219',typeInteger,"",flagsSingleParameter,
					              
				"ctl220",'0220',typeInteger,"",flagsSingleParameter,
				"ctl221",'0221',typeInteger,"",flagsSingleParameter,
				"ctl222",'0222',typeInteger,"",flagsSingleParameter,
				"ctl223",'0223',typeInteger,"",flagsSingleParameter,
				"ctl224",'0224',typeInteger,"",flagsSingleParameter,
				"ctl225",'0225',typeInteger,"",flagsSingleParameter,
				"ctl226",'0226',typeInteger,"",flagsSingleParameter,
				"ctl227",'0227',typeInteger,"",flagsSingleParameter,
				"ctl228",'0228',typeInteger,"",flagsSingleParameter,
				"ctl229",'0229',typeInteger,"",flagsSingleParameter,
					              
				"ctl230",'0230',typeInteger,"",flagsSingleParameter,
				"ctl231",'0231',typeInteger,"",flagsSingleParameter,
				"ctl232",'0232',typeInteger,"",flagsSingleParameter,
				"ctl233",'0233',typeInteger,"",flagsSingleParameter,
				"ctl234",'0234',typeInteger,"",flagsSingleParameter,
				"ctl235",'0235',typeInteger,"",flagsSingleParameter,
				"ctl236",'0236',typeInteger,"",flagsSingleParameter,
				"ctl237",'0237',typeInteger,"",flagsSingleParameter,
				"ctl238",'0238',typeInteger,"",flagsSingleParameter,
				"ctl239",'0239',typeInteger,"",flagsSingleParameter,
					              
				"ctl240",'0240',typeInteger,"",flagsSingleParameter,
				"ctl241",'0241',typeInteger,"",flagsSingleParameter,
				"ctl242",'0242',typeInteger,"",flagsSingleParameter,
				"ctl243",'0243',typeInteger,"",flagsSingleParameter,
				"ctl244",'0244',typeInteger,"",flagsSingleParameter,
				"ctl245",'0245',typeInteger,"",flagsSingleParameter,
				"ctl246",'0246',typeInteger,"",flagsSingleParameter,
				"ctl247",'0247',typeInteger,"",flagsSingleParameter,
				"ctl248",'0248',typeInteger,"",flagsSingleParameter,
				"ctl249",'0249',typeInteger,"",flagsSingleParameter,
					              
				"ctl250",'0250',typeInteger,"",flagsSingleParameter,
				"ctl251",'0251',typeInteger,"",flagsSingleParameter,
				"ctl252",'0252',typeInteger,"",flagsSingleParameter,
				"ctl253",'0253',typeInteger,"",flagsSingleParameter,
				"ctl254",'0254',typeInteger,"",flagsSingleParameter,
				"ctl255",'0255',typeInteger,"",flagsSingleParameter,
					
				#if defined(PLUGINGALAXY3)
					"txt000",'t000',typeChar,"",flagsSingleParameter,
					"txt001",'t001',typeChar,"",flagsSingleParameter,
					"txt002",'t002',typeChar,"",flagsSingleParameter,
					"txt003",'t003',typeChar,"",flagsSingleParameter,
					"txt004",'t004',typeChar,"",flagsSingleParameter,
					"txt005",'t005',typeChar,"",flagsSingleParameter,
					"txt006",'t006',typeChar,"",flagsSingleParameter,
					"txt007",'t006',typeChar,"",flagsSingleParameter,
					"txt008",'t006',typeChar,"",flagsSingleParameter,
					"txt009",'t006',typeChar,"",flagsSingleParameter,
					"txt010",'t010',typeChar,"",flagsSingleParameter,
					"txt011",'t011',typeChar,"",flagsSingleParameter,
					"txt012",'t012',typeChar,"",flagsSingleParameter,
					"txt013",'t013',typeChar,"",flagsSingleParameter,
					"txt014",'t014',typeChar,"",flagsSingleParameter,
					"txt015",'t015',typeChar,"",flagsSingleParameter,
					"txt016",'t016',typeChar,"",flagsSingleParameter,
					"txt017",'t017',typeChar,"",flagsSingleParameter,
					"txt018",'t018',typeChar,"",flagsSingleParameter,
					"txt019",'t019',typeChar,"",flagsSingleParameter,
					"txt020",'t020',typeChar,"",flagsSingleParameter,
					"txt021",'t021',typeChar,"",flagsSingleParameter,
					"txt022",'t022',typeChar,"",flagsSingleParameter,
					"txt023",'t023',typeChar,"",flagsSingleParameter,
					"txt024",'t024',typeChar,"",flagsSingleParameter,
					"txt025",'t025',typeChar,"",flagsSingleParameter,
					"txt026",'t026',typeChar,"",flagsSingleParameter,
					"txt027",'t027',typeChar,"",flagsSingleParameter,
					"txt028",'t028',typeChar,"",flagsSingleParameter,
					"txt029",'t029',typeChar,"",flagsSingleParameter,
					"txt030",'t030',typeChar,"",flagsSingleParameter,
					"txt031",'t030',typeChar,"",flagsSingleParameter
				#endif
					
			}
		},
		{													/* non-filter plug-in class here */
		},
		{													/* comparison ops (not supported) */
		},
		{													/* any enumerations */
		}
	}
};
// #endif

#pragma warning(pop)
