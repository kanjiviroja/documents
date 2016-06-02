function GetElqContentPersonalizationValue(strDataField){ var strTemp = '';        if(strDataField == 'V_Email_Address' || strDataField == 'Email_Address'){strTemp = 'kanji.viroja\x40slscorp.com';}
        if(strDataField == 'V_Eloqua_Contact_ID1p' || strDataField == 'Eloqua_Contact_ID1p'){strTemp = 'CFSSC000002170963';}
	return strTemp; } if (this.SetElqContent){ SetElqContent(); }