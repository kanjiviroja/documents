//Variables for debugging
var s_log = 'start';
var s_vid = ''; // Used to set Adobe Analytics Variable s.eVar15
var s_download = '';
var cnt = 0;
var s_searchLog = "";
var s_searchCount = 0;

var searchTerm = ''; // Used to set Adobe Analytics variable s.eVar31
var contentId = ''; // Used to set Adobe Analytics variable s.eVar27
var currentSpace = ''; //Used to set Adobe Analytics site section variables s.prop2, s.prop3, s.prop8, s.prop9, s.prop10, s.prop11
var parentSpace = ''; //Used to set Adobe Analytics variable s.eVar25
var siteSection = ''; //Used to set Adobe Analytics site section variables s.prop2, s.prop3, s.prop8, s.prop9, s.prop10, s.prop11
var prevRef = ''; // document.referrer
var tid = ''; //Used to set Adobe Analytics variable s.eVar23
var activityType = ''; //Used to set Adobe Analytics variable s.eVar30
var category = ''; //Used to set Adobe Analytics variable s.eVar28
var contentType = ''; //Used to set Adobe Analytics variable s.prop1
var tags = ''; //Used to set Adobe Analytics variable s.eVar29
var title = ''; //used to set Adobe Analytics Variable s.pageName
var pageNameLen = 100; //Used in setting Adobe Analytics Variable s.pageName
var pageURL = ''; //used to set Adobe Analytics Variable s.pageURL
var channel = ''; //Used in logic to set various Adobe Analytics Variables, including s.prop1, s.prop2
var strKey = ''; //Used in logic to search for specific strings in url
var communityURL = '';
var tab = ''; //Used to set Adobe Analytics Variable s.eVar26
var userid; //Currently not used in Adobe Analytics
var username; //used to set Adobe Analytics Variable s.eVar32
var isErrorPage = false; //Used to determine when to set Adobe Analytics variable for Error page
var searchIn = "Freescale Community"; 

var setTimeSearch = 0;
var setIntAuth = 0;
var setIntPlace = 0;

var contentFindingSet = false;

var linkType = "UNKNOWN";
var elqTryI = false;

//Google Code for Remarketing Tag
//--------------------------------------------------
//Remarketing tags may not be associated with personally identifiable information or placed on pages related to sensitive categories. See more information and instructions on how to setup the tag on: http://google.com/ads/remarketingsetup
//---------------------------------------------------

//<script type="text/javascript">
//<![CDATA[ */
//var google_conversion_id = 1066477406;
//var google_custom_params = window.google_tag_params;
//var google_remarketing_only = true;
/* ]]> */
//</script>
//<script type="text/javascript" src="//www.googleadservices.com/pagead/conversion.js">
//</script>
//<noscript>
//<div style="display:inline;">
//<img height="1" width="1" style="border-style:none;" alt="" src="//googleads.g.doubleclick.net/pagead/viewthroughconversion/1066477406/?value=0&amp;guid=ON&amp;script=0"/>
//</div>
//</noscript>


//For Historical Eloqua Contact Lookup

document.write("<script src='//www.freescale.com/files/js/elqNow/elqwebanalytics.js'></script>"); //Used to set Eloqua Variable for historical data purposes s.eVar38 in Adobe Tag Manager Code

//For getting Freescale Visitor Id
document.write("<script src='//www.freescale.com/webapp/visitorIdLookup.jsp'></script>"); //Used to set Adobe Analytics variable s.eVar15
document.write("<script src='//style.nxp.com/files/abstract/misc/blueconic.js'></script>");


//eloqua scripts
document.write("<script src='//www.freescale.com/files/js/elqNow/elqCfg.js'></script>");
document.write("<script src='//www.freescale.com/files/js/elqNow/elqFCS.js'></script>");
var digitalData={};
digitalData.pageInfo={};
digitalData.userInfo={};
digitalData.siteInfo={};
digitalData.communityInfo={};
digitalData.searchInfo={};
$j(document).ready(function() {
	s_log = 'step 1';

	searchTerm = '';
	contentId = '';
	currentSpace = $j('.j-menu.j-breadcrumb-popover ul li a.last-child').text(); //No Updates Required
	parentSpace = $j('.j-menu.j-breadcrumb-popover ul ul li a:not(.last-child)').text(); //No updates Required
	prevRef = encodeURI(document.referrer); 
	tid = extractQP('tid'); 
	activityType = 'view';
	category = '';
	contentType = '';
	tags = '';
	title = '';
	pageURL = '';
	channel = '';
	strKey = '';
	communityURL = '';
	tab = '';
	userid = '';
	username = '';
	isErrorPage = false;


	if(prevRef.indexOf('post%21input.jspa') != -1)
		activityType = 'create';
	else if(prevRef.indexOf('edit') != -1)
		activityType = 'edit';

	if($j('#j-search-input').length>0){
		searchTerm = $j('#j-search-input').val(); //No updates Required
	}

	if($j('header#j-compact-header img.jive-avatar').length > 0){ //PreCloud Migration if($j('header#j-header img.jive-avatar').length > 0){ //No updates Required
		userid = 0;
		username = $j('header#j-compact-header img.jive-avatar').attr('data-username'); //PreCloud Migration username = $j('header#j-header img.jive-avatar').attr('data-username'); //No updates Required
	}else{
		userid = 0;
		username = "Guest";
	}

	if(document.title == "Error | Freescale Community"){ //No updates Required
		isErrorPage = true;
	}else{
		contentId = getContentId(); //No Updates Required
	}

	s_log = 'getContentId';

	currentSpace = trim1(currentSpace); //No Updates Required
	parentSpace = trim1(parentSpace); //No Updates Required

	if(($j('nav#j-placeNav').length != 0 || $j('nav ul.j-tabbar').length != 0) && $j('#j-globalNav .active').length <= 0){   // PreCloud Migration - if($j('nav.j-bigtab-nav').length != 0 && $j('#j-globalNav .active').length <= 0){
		title = $j('header.j-page-header h1 a.j-placeName').text(); // PreCloud Migration - title = $j('header.j-page-header h1 a.font-color-normal').text();
		// PreCloud Migration - tab = $j('ul.j-tabbar li.j-tab-selected').text();
		if($j('ul#j-placeMainNav li.j-tab-selected').length > 0){
			tab = $j('ul#j-placeMainNav li.j-tab-selected').text();
		}else if($j('ul.j-tabbar li.j-tab-selected').length>0){
			tab = $j('ul.j-tabbar li.j-tab-selected').text().trim();
		}

		tab = tab.replace(/\s+/g, ' ');

		if(typeof elqFCS == 'function'){
			formElqUrl(title,parentSpace,tab,contentId,'',currentSpace,'','',activityType,searchTerm,tid,userid,username);
		}
	}else{
		if(($j('#jive-body-main').length > 0 || $j('#jive-body').length>0) && $j('#j-globalNav .active').length <= 0 && $j('.jive-create-large').length <=0){
			title = $j('.jive-content header h1').text(); //No updates Required
			contentType = $j('#jive-body-intro .j-context a').first().text(); //No Updates Required
			$j('span.jive-thread-post-details-tags a span').each(function(){ // PreCloud Migration - tags = $j('span.jive-thread-post-details-tags a').map(function() { return this.text; }).get().join(',');
				var txt = $j(this).text().trim();
				if(txt != ''){
					if(tags != '') tags += ",";  
					tags = tags + txt;
				}
			});

			category = $j("span.jive-content-footer-item:not(.jive-content-footer-tags,.jive-vid-tags) span:not('.jive-video-views-count')").text(); //No Updates Required

			title = title.replace(/\s+/g, ' ');
			tags = tags.replace(/\s+/g, '');
			category = category.replace(/\s+/g, ' ').replace('Categories:','');
			category = category.replace(/,/g, ';');
			contentType = contentType.replace(/\s+/g, ' ');
			contentType = contentType.replace('More ','');

			if(typeof elqFCS == 'function'){
				formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
			}
		}else if($j('#j-globalNav .active').length > 0){ //No Updates Required
			var menuItem  = $j('#j-globalNav .active').attr('id'); //No Updates Required

			if(menuItem == 'jive-nav-link-home'){ // No Updates Required
				tab = 'Home';
				title = 'Home';
			}
			if(menuItem == 'jive-nav-link-work'){ // No Updates Required
				tab = 'Content';
				title = 'Content';
			}
			if(menuItem == 'jive-nav-link-conx'){ // No Updates Required
				tab = 'People';
				title = 'People';
			}
			if(menuItem == 'jive-nav-link-places'){ // No Updates Required
				tab = 'Places';
				title = 'Places';
			} 

			currentSpace = "Freescale Community";
			activityType = 'view';

			if(typeof elqFCS == 'function'){
				formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
			}

		}
	}

	$j('.acclaim-like-container').click(function(event){
		activityType = $j(this).find('.jive-acclaim-likelink').first().attr('data-command');

		/*Added for Migration - Map to values used prior to migration */
		if(activityType == 'credit'){
			activityType = 'like';
		}else if(activityType == 'uncredit'){
			activityType = 'unlike';
		}

		if(typeof activityType == 'undefined')
			return;

		var $this = $j(event.target);
		if($this.attr('data-command') == 'showAcclaim') //Pre Cloud Migration - if($this.attr('data-command') == 'showLikes')
			return;

		if(typeof elqFCS == 'function'){
			formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
		}
		// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
		// PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );

		trackCommunityActivity(activityType);
	});

	$j('.j-js-follow-controls').click(function(){ //No updates Required
		setTimeout(function(){
			if($j('#follow-in-menu').length != 0){
				$j('.j-stop-following').click(function(){
					activityType = "stoppedFollowing";
					if(typeof elqFCS == 'function'){
						formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
					}
					// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
					// PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
					trackCommunityActivity(activityType);					
				});
			}	
		},200);

		if($j(this).find('a[id$="-startFollowing"]').css("display") != "none"){
			activityType = "startedFollowing";
			if(typeof elqFCS == 'function'){
				formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
			}
			// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
			//PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
			trackCommunityActivity(activityType);						
		}
	});		

	$j('#jive-link-thread-stopFollowing').click(function(){
		activityType = "stoppedFollowing";
		if(typeof elqFCS == 'function'){
			formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
		}
		// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
		//PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
		trackCommunityActivity(activityType);
	});

	/* Pre Cloud Migration
		$j('.js-link-delete').click(function(){
			activityType = "delete";
			if(typeof elqFCS == 'function'){
				formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
			}
			// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
			$j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
		});
	 */

	$j('.js-link-delete').click(function(){
		activityType = "delete";
		if(typeof elqFCS == 'function'){
			formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
		}
		// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
		// PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
	});

	$j('#jive-link-thread-startTracking').click(function(){
		activityType = "startedTracking";
		if(typeof elqFCS == 'function'){
			formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
		}
		// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
		// PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
		trackCommunityActivity(activityType);
	});

	$j('#jive-link-thread-stopTracking').click(function(){
		activityType = "stoppedTracking";
		if(typeof elqFCS == 'function'){
			formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
		}
		// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
		// PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );

		trackCommunityActivity(activityType);
	});

	$j('.discussionAdd').click(function(){		
		$j("body").delegate(".jive-form-button-save", "click", function(){
			var elem = $j(this).parents('#postform');

			activityType = "reply";
			if(typeof elqFCS == 'function'){
				formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
			}
			// PreCloud Migration - Webtrends Removal $j('meta[id=community_activity_type]').attr('content', activityType);
			// PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
		});
	});
	$j('div.j-contained-tabs-place.j-browse-content div.j-sidebar-left nav.j-second-nav-categories li').click(function(){
		category = $j(this).find('.nav-label').text();
		if(typeof elqFCS == 'function'){
			formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username);
		}
		// PreCloud Migration - Webtrends Removal $j('meta[id=community_category]').attr('content', category);
		// PreCloud Migration - Webtrends Removal $j.getScript( "/themes/freescale_theme_v2_0/scripts/FSL_COMM_SMARTSOURCE_TAG.js" );
	});


	s_log = 'before adobe call';
	pageURL = document.URL;
	if (contentType == ''){
		contentType = "General";
	}

	siteSection = '';
	$j('#js-place-parents-container ul ul li a:not(.last-child)').each(function(){ // PreCloud Migration - $j('.j-menu.j-breadcrumb-popover ul ul li a:not(.last-child)').each(function(){
		var txt = $j(this).text().trim(); //No Updates Required
		if(txt != ''){
			if(siteSection != '') siteSection += ","; 
			siteSection = siteSection + txt;
		}
	});

	
	$j.getScript( "//assets.adobedtm.com/f2bd0c145f764c0e7b6dae5f9269061e38fabacd/satelliteLib-2fb7098ae4e0938976f1eee822973607d3f48c60.js", function(){

		if(typeof getFslVisitorIDForAnalytics == 'function'){				
			s_vid = getFslVisitorIDForAnalytics();
		}

		if($j('#j-search-input').length>0){ //No Updates Required
			s_log = 'in search';
			title = "Search";
			channel = "Search";
			contentType = "Search";
		}else{
			if(window.document.title.indexOf("...")==-1||title==''){
				title = window.document.title;
			}
			title = title.replace(' | Freescale Community','');
			title = title.replace('Space: ','');

			strKey = '/people/';
			if(pageURL.indexOf(strKey) > -1){
				channel = "People";
			}

			if(currentSpace == ""){
				strKey = '/community/';
				if(pageURL.indexOf(strKey) > -1){
					if(pageURL.indexOf("/tags#/") > -1){
						/* Pre Cloud

								var pageHead = $j("header.j-page-header > h1 > a").first().text();
								currentSpace = $j.trim(pageHead.replace("Browse all tags in",""));

								if($j("header.j-page-header > h1 > span.j-page-header-parent > a").length > 0){
									parentSpace = $j("header.j-page-header > h1 > span.j-page-header-parent > a").first().text();
									if(parentSpace == currentSpace){
										parentSpace = "";
									}
								}							

						 */
						var pageHead = $j("header.j-page-header > h1").first().text().trim(); // Pre Cloud Migration var pageHead = $j("header.j-page-header > h1 > a").first().text();
						pageHead = $j.trim(pageHead.replace("Browse all tags in",""));
						if($j("header.j-page-header > h1 > span.j-page-header-parent").length > 0){	//if($j("header.j-page-header > h1 > span.j-page-header-parent > a").length > 0){
							parentSpace = $j("header.j-page-header > h1 > span.j-page-header-parent").first().text().trim(); //parentSpace = $j("header.j-page-header > h1 > span.j-page-header-parent > a").first().text();
							currentSpace=$j.trim(pageHead.replace(parentSpace,""));
							parentSpace = $j.trim(parentSpace.replace("in ",""));
						}
						else{
							currentSpace = pageHead;
						}
					}else{
						communityURL = pageURL.substr(pageURL.indexOf(strKey) + strKey.length);

						if(communityURL.indexOf('/') > -1)
							channel = communityURL.substring(0,communityURL.indexOf('/'));
					}
				}
			}
		}

		var errorPageURL = "";
		if(isErrorPage){
			parentSpace = "";
			contentType = "Error";
			channel = "Error";
			title = "Error";
			if($j("div#jive-body > div#jive-body-intro > h1").length > 0){  //No updates required
				contentId = contentType + ":" + $j("div#jive-body > div#jive-body-intro > h1").first().text(); //No updates required
				title = $j("div#jive-body > div#jive-body-intro > h1").first().text(); //No updates required
			}

			var domain = "";

			/* Pre Cloud Migration
				if(document.URL.indexOf("community-uat.freescale.com") > 0){
					domain = "http://community-uat.freescale.com/";
				}else{
					domain = "http://community.freescale.com/";
				}
			 */
			domain = "http://freescale.jiveon.com/";

			errorPageURL = domain + contentId.replace(":", "_").replace(" ", "_").toLowerCase();			
		}

		initContentFinding();

		if(channel == "Search"){
			if($j('dd').length > 0){
				if($j('dd.j-community-bridged').hasClass('j-active')){
					searchIn = "Freescale Website";
				}

				$j('dd').click(function(){
					setTimeout(checkSearchIn, 10);
				});
			}
			setTimeSearch = setTimeout(initSearchTracking, 500);
		}else{

			s_log = 'get s object';

			digitalData.pageInfo.pageLoadRule="CommunityGeneral";
			if(!isErrorPage){
				digitalData.pageInfo.pageURLClean = getPageURLForAnalytics();
			}else{
				digitalData.pageInfo.pageURLClean = errorPageURL;
			}
			digitalData.pageInfo.pageName  = truncateString("Community: "+title, pageNameLen);

			if(channel == "Search"){
				digitalData.pageInfo.pageType = "Community Search";
				digitalData.pageInfo.siteSection2 = "Community Search";
			}else{				
				digitalData.pageInfo.pageType = "Community: "+contentType;

				if(channel!=''){					
					digitalData.pageInfo.siteSection2 = "Community: "+ channel;
				}else{
					if(currentSpace != ""){
						if(siteSection != ''){
							var siteSections = siteSection.split(',');
							var numOfParents = siteSections.length;
							for(ss=0;ss<numOfParents;ss++){
								if(ss == 0) digitalData.pageInfo.siteSection2 = 'Community: '+ siteSections[ss];
								if(ss == 1) digitalData.pageInfo.siteSection3 = 'Community: '+ siteSections[ss];
								if(ss == 2) digitalData.pageInfo.siteSection4 = 'Community: '+ siteSections[ss];
								if(ss == 3) digitalData.pageInfo.siteSection5 = 'Community: '+ siteSections[ss];
								if(ss == 4) digitalData.pageInfo.siteSection6 = 'Community: '+ siteSections[ss];
								if(ss == 5) digitalData.pageInfo.siteSection7 = 'Community: '+ siteSections[ss];
							}

							if(numOfParents == 1) digitalData.pageInfo.siteSection3 = 'Community: '+ currentSpace;
							if(numOfParents == 2) digitalData.pageInfo.siteSection4 = 'Community: '+ currentSpace;
							if(numOfParents == 3) digitalData.pageInfo.siteSection5 = 'Community: '+ currentSpace;
							if(numOfParents == 4) digitalData.pageInfo.siteSection6 = 'Community: '+ currentSpace;
							if(numOfParents == 5) digitalData.pageInfo.siteSection7 = 'Community: '+ currentSpace;			
						}else{
							digitalData.pageInfo.siteSection2 = "Community: "+ currentSpace;
						}
					}else{
						digitalData.pageInfo.siteSection2 = "Community: "+ "NXP Community";
					}
				}						
			}				
			digitalData.pageInfo.siteSection1= "Community";

			digitalData.pageInfo.pageEventPathing = "D=pageName";
			digitalData.pageInfo.contentFinding  = getContentFinding(document.referrer, document.URL);

			if(typeof s_vid != 'undefined' && s_vid != '') 
				digitalData.userInfo.fslVisitorID = s_vid;

			digitalData.pageInfo.assetID = "D=pageName";
			digitalData.siteInfo.sitePlatform = "full";
			digitalData.siteInfo.lang = "en";
			digitalData.pageInfo.localTitle = "D=pageName";
			digitalData.pageInfo.trackingID = getQryParamForAnalytics(document.URL, "tid");
			digitalData.communityInfo.parentCommunity = parentSpace;
			digitalData.communityInfo.tab = tab;
			digitalData.communityInfo.contentID = contentId;
			digitalData.communityInfo.category = category;
			digitalData.pageInfo.tags = tags;
			digitalData.communityInfo.activityType = activityType;
			digitalData.communityInfo.searchKeyword = searchTerm;
			digitalData.userInfo.userName = username;
			//s.eVar33 = userid; Not used
			//	s.t();
			//_satellite.track('fslCommunity');
			_satellite.pageBottom();
			s_log = 'page tracked';

		}
	});

	$j('#campaignId').click(function(){
		var el = document.getElementById("campaignId");
		var attrList = '';
		var attrName = '';
		for (var i=0, attrs=el.attributes, l=attrs.length; i<l; i++){
			attrName=attrs.item(i).nodeName;
			if(attrName == 'id')
				continue;
			attrList += attrName+'='+$j('#campaignId').attr(attrName)+'&';
		}
		attrList += 'Title='+title+'&parentSpace='+parentSpace+'&currentSpace='+currentSpace+'&link='+link;
		if(typeof elqFCS == 'function'){
			elqFCS(attrList);
		}
	});


});

function formElqUrl(title,parentSpace,tab,contentId,contentType,currentSpace,category,tags,activityType,searchTerm,tid,userid,username){
	var elqURL=window.location.href;
	if(elqURL.indexOf('?') != -1){
		elqURL = elqURL.substring(0,elqURL.indexOf('?'));
	}
	var assetID = contentId;
	if(assetID == '')
		assetID = '1';
	//var webActivityUrl = "//www.freescale.com/webapp/history/recordActivity.sp?assetID="+assetID+"&assetType=COMMUNITY_SITE&commandID=RECORD_WEB_ACTIVITY&referrerUrl=";
	var queryString = '?Title='+title+
	'&parentSpace='+parentSpace+
	'&tab='+tab+
	'&contentId='+contentId+
	'&contentType='+contentType+
	'&currentSpace='+currentSpace+
	'&category='+category+
	'&tags='+tags+
	'&activityType='+activityType+
	'&searchTerm='+searchTerm+
	'&userid='+userid+
	'&username='+username;
	if(tid != '')
		queryString += '&tid='+tid;	
	//webActivityUrl += encodeURIComponent(elqURL + queryString);
	elqURL = elqURL + queryString;
	elqFCS(elqURL);
	// Removed during the migration to the cloud
	//$j("#j-globalNav-bg").after("<iframe style='display:none' src="+webActivityUrl+"/>");
}


function getContentId(){
	pageURL = document.URL;
	var idx = pageURL.lastIndexOf("/");	
	var tempURL = pageURL.substring(0,idx);

	var contentId = pageURL.substring(idx+1);
	//Added for cloud migration 4/28/2015
	contentId=contentId.split("?")[0];

	var type = "";

	if(contentId.indexOf("#") > -1){
		var cp = $j.trim(contentId).split("#");
		if(cp.length > 1){
			contentId = cp[cp.length-1] != "" ? cp[cp.length-1] : cp[cp.length-2];
		}
	}

	if(pageURL.indexOf("/thread/") > -1){
		type = "thread";
	}

	if(pageURL.indexOf("/message/") > -1){
		type = "message";
	}	

	if(pageURL.indexOf("/polls/") > -1){
		type = "poll";
	}

	if(pageURL.indexOf("/ideas/") > -1){
		type = "idea";
	}

	if(pageURL.indexOf("/events/") > -1){
		type="events"; //type = "idea";
	}	
	/*Added Videos for Migration */
	if(pageURL.indexOf("/videos/") > -1){
		type="videos";
	}	

	if(pageURL.indexOf("/blog/") > -1 && pageURL.indexOf("/tags#/") == -1 && pageURL.indexOf("/blog/create-post.jspa") == -1){
		type = "blog";
		if($j(".jive-content-blog").length > 0)
			contentId = $j(".jive-content-blog").attr("data-object-id"); // PreCloud Migration contentId = $j(".jive-content-blog").attr("aria-labeledby").replace("heading-", "");
		// PreCloud Migration contentId = contentId.replace("update-post.jspa?ID=","");
	}

	if(pageURL.indexOf("/tags#/") > -1){
		type = "tag";
		contentId = contentId.replace("?tags=", "");
	}

	if(pageURL.indexOf("/docs/") > -1){
		type = "doc";
		contentId = contentId.replace("DOC-", "");
	}

	if(type != ""){
		contentId = type + ":" + contentId;
	}else{
		contentId = "";
	}	

	return  contentId;	
}

function getPageURLForAnalytics() {
	staticPageURL = document.URL.split("?")[0];
	return staticPageURL;
}

function trackCommunityActivity(communityActivity){
	if (typeof s_account != 'undefined') {
		var s1=s_gi(s_account);

		s1.linkTrackVars = 'channel,prop2,prop3,prop6,prop8,prop9,prop10,' +
		'prop11,prop13,prop14,prop16,prop21,prop74,eVar1,eVar2,eVar3,' +
		'eVar4,eVar5,eVar6,eVar15,eVar16,eVar17,eVar18,eVar19,' +
		'eVar25,eVar26,eVar27,eVar28,eVar29,eVar30,eVar31,eVar32,eVar33,' +
		'eVar34,eVar35,eVar38,eVar39,eVar48,eVar51,eVar52,eVar53,events';
		s1.linkTrackEvents = 'event32';
		s1.prop16 = communityActivity + ':' + title;
		s1.eVar30 = communityActivity;
		s1.events = 'event32';
		s.tl(this,'o',communityActivity + ':' + title);
		s1.events = '';
	}
}

function trim1 (str) {
	return str.replace(/^\s\s*/, '').replace(/\s\s*$/, '');
}

function extractQP(name)
{
	var regexS = "[\\?&]"+name+"=([^&#]*)";
	var regex = new RegExp( regexS );
	var tmpURL = window.location.href;
	var results = regex.exec( tmpURL );
	if( results == null )
		return "";
	else
		return results[1];
} 

function checkSearchIn(){
	var searchInNew = '';
	if($j('dd.j-community-bridged').hasClass('j-active')){
		searchInNew = "Freescale Website";
	}else{
		searchInNew = "Freescale Community";
	}
	if(searchInNew != searchIn){
		searchIn = searchInNew;
		trackSearchForAnalytics();
	}
}

function getContentFinding(referringUrl, currentUrl, isDownload) {
	if(typeof isDownload == 'undefined') isDownload = false;
	var contentFinding = "";

	contentFinding = getContentFindingFromCookie();
	if(contentFinding != "") return contentFinding;
	
	contentFinding = getContentFindingFromParam(currentUrl, referringUrl);
	if(contentFinding != "") return contentFinding;	
	
	contentFinding = getContentFindingFromReferrer(referringUrl, isDownload);
	if(contentFinding != "") return contentFinding;		
	
	contentFinding = getContentFindingFromTid(currentUrl);
	
	return contentFinding;
}

function getContentFindingFromCookie(){
	return getAdobeCookie('contentfinding');
}

function getContentFindingFromParam(currentUrl, referringUrl){
	var reffererPageCategory = getReffererPageCategory();
	var pageCategory = getPageCategory();
	
	var qry_param = "";	
	var url_parts = currentUrl.split("?");
	if(url_parts.length>1) 
		qry_param = url_parts[1];
	else
		return "";
	
	if(qry_param.indexOf("site_preference=normal") != -1) return "Footer - Select Fullsite";
	
	if(qry_param.indexOf("tid=FSHBNR") != -1) {	
		var refererrPathName = referringUrl.replace(/^[^:]+:\/\/[^/]+/, '').replace(/#.*/, '');
		var fileName = refererrPathName.substring(refererrPathName.lastIndexOf("/") + 1);
		if(fileName.length < 2 || fileName.indexOf("index") != -1 || (fileName.indexOf(".") < 0 && fileName.indexOf("login") < 0)) {
			return "Homepage banner";
		}else
			return "";
	}
	
	if(qry_param.indexOf("fr=p") != -1 && referringUrl != "") return "Personal Recommendations";

	//No Longer Used : start
	if(qry_param.indexOf("fr=gtl") != -1 && referringUrl != "") 		
		return "General Recommendations on Downloads/Software and Tools Tab";
		
	if(qry_param.indexOf("fr=gdc") != -1 && referringUrl != "")		
			return "General Recommendations on Documentation Tab";

	if(qry_param.indexOf("fr=g") != -1 && referringUrl != "")		
			return "General Recommendations on bottom of Summary Page";
	//No Longer Used : end
	
	if(qry_param.indexOf("fsrch=1") != -1 && referringUrl != "")
			return "Keyword Search";
	
	if(qry_param.indexOf("fpsp=1") != -1 && referringUrl != "" && referringUrl.indexOf("/products/") != -1){
		if(reffererPageCategory == "PSP" || reffererPageCategory == "TSP" || reffererPageCategory == "SSP" || reffererPageCategory == "RDSP")
			return "PSP";
	} 
	if(qry_param.indexOf("fasp=1") != -1 && referringUrl != "") return "ASP";
	
	if(qry_param.indexOf("hdr=1") != -1) return "Header";
	if(qry_param.indexOf("hdr=sl") != -1) return "Header - Select Language";
	if(qry_param.indexOf("ftr=sm") != -1) return "Footer - Select Mobile";
	if(qry_param.indexOf("ftr=1") != -1) return "Footer";	
	
	return "";
}

function getPageCategory(){
	if(typeof digitalData == "undefined") return "";
	if(typeof digitalData.pageInfo == "undefined") return "";
	if(typeof digitalData.pageInfo.pageCategory == "undefined") return "";
	return digitalData.pageInfo.pageCategory;
}

function getPageCodeID(){
	if(typeof digitalData == "undefined") return "";
	if(typeof digitalData.pageInfo == "undefined") return "";
	if(typeof digitalData.pageInfo.pageCodeID == "undefined") return "";
	return digitalData.pageInfo.pageCodeID;
}

function getReffererPageCategory(){
	return getAdobeCookie("referrerPageCategory");
}

function getReffererPageCode(){
	return getAdobeCookie("referrerPageCode");
}

function getContentFindingFromReferrer(referringUrl, isDownload){	
	var reffererPageCategory = getReffererPageCategory();
	var reffererPageCode = getReffererPageCode();
	var pageCategory = getPageCategory();
	var pageCode = getPageCodeID();
	
	if(typeof referringUrl == 'undefined')
		referringUrl = "";
	if(referringUrl.split("?")[0].indexOf("/parametricSelector.sp") != -1 && (pageCategory != 'PARAMETRIC SEARCH' || isDownload)){
		if(pageCategory != "KEYWORD SEARCH" && pageCategory != "SOFTWARE SEARCH")
			return "Parametric Search";
	} 
	
	if(referringUrl.indexOf("/packages/search?") != -1 && (pageCategory != 'PACKAGE SEARCH' || isDownload)){
		return "Package Search";
	}

	var searchUrl = "/search?";
	if(typeof URL_DOMAIN != 'undefined')
		searchUrl = URL_DOMAIN + searchUrl;	
	
	if(referringUrl.indexOf(searchUrl) != -1 && referringUrl.indexOf("/packages/search?") == -1 && (pageCategory != 'KEYWORD SEARCH' || isDownload)){
		if(pageCategory != "PARAMETRIC SEARCH" && pageCategory != "SOFTWARE SEARCH")
			return "Keyword Search";
	}

	if(referringUrl.split("?")[0].indexOf("software-center/library.jsp") != -1 && (pageCategory != 'SOFTWARE SEARCH' || isDownload)){
		if(pageCategory != "PARAMETRIC SEARCH" && pageCategory != "KEYWORD SEARCH")
			return "Software Search";
	}
	
	var chemicalContentURL = "/chemical-content/";
	var chemicalContentIMDSURL = "/chemical-content/imds";
	var chemicalContentSearchURL = "/chemical-content/search";
	if(referringUrl.indexOf(chemicalContentURL) != -1 && referringUrl.indexOf(chemicalContentIMDSURL) == -1 && referringUrl.indexOf(chemicalContentSearchURL) == -1)  
		return "Chemical Content Page";
	
	if(referringUrl.split("?")[0].indexOf(chemicalContentSearchURL) != -1 && (pageCategory != 'CHEMICAL CONTENT SEARCH' || isDownload)){
		if(pageCategory != "PARAMETRIC SEARCH" && pageCategory != "KEYWORD SEARCH")
			return "Chemical Content Search";
	}
	
	if(isDownload){
		reffererPageCategory = pageCategory;
		pageCategory = "DOWNLOAD";
	}
	
	if(reffererPageCategory == 'PARAMETRIC SEARCH' && pageCategory != 'PARAMETRIC SEARCH')
		return "Parametric Search";	
	
	if(reffererPageCategory == 'KEYWORD SEARCH' && pageCategory != 'KEYWORD SEARCH')
		return "Keyword Search";	

	if(reffererPageCategory == 'SOFTWARE SEARCH' && pageCategory != 'SOFTWARE SEARCH')
		return "Software Search";

	if(reffererPageCategory == 'PACKAGE SEARCH' && pageCategory != 'PACKAGE SEARCH')
		return "Package Search";	
	
	if(reffererPageCategory == 'PART FINDER' && pageCategory != 'PART FINDER')
		return "Cross Check: Part Finder";
	
	if(reffererPageCategory == 'COMPETITOR XREF' && pageCategory != 'COMPETITOR XREF')
		return "Cross Check: Competitor Cross Reference";
	
	if(reffererPageCategory == 'RF DRIVER' && pageCategory != 'RF DRIVER')
		return "RF Suggested Driver";
	
	if(reffererPageCategory == 'RF PREDRIVER' && pageCategory != 'RF PREDRIVER')
		return "RF Suggested Predriver";
	
	if(reffererPageCategory == 'PSP' || reffererPageCategory == 'TSP' || reffererPageCategory == 'RDSP' || reffererPageCategory == 'SSP'){
		if(reffererPageCode != pageCode) return "PSP";
	}
	
	if(reffererPageCategory == 'ASP' && reffererPageCode != pageCode) return "ASP"; 

	if(reffererPageCategory == 'VIDEO VAULT' && pageCategory != 'VIDEO VAULT')
		return "Video Vault";	

	if(reffererPageCategory == 'VIDEO CHANNEL' && pageCategory != 'VIDEO CHANNEL')
		return "Video Channel";	

	if(reffererPageCategory == 'VIDEO' && reffererPageCode != pageCode)
		return "Video Summary Page";
	
	if(reffererPageCategory == 'PIP')
		if(reffererPageCode != pageCode) return "Package Summary Page";

	if(reffererPageCategory == 'MORE INFO')
		return "More Info";	

	if(reffererPageCategory == 'CHEMICAL CONTENT')
		return "Chemical Content Page";

	if(reffererPageCategory == 'CHEMICAL CONTENT SEARCH' && pageCategory != 'CHEMICAL CONTENT SEARCH')
		return "Chemical Content Search";	
	
	return "";
}

function getContentFindingFromTid(currentUrl){
	var tid = getQryParamForAnalytics(currentUrl, 'tid');
	if(tid == "") return "";
 	if(tid == 'van') return "Vanities";
 	if(tid == 'persHis') return "Personal History/Favorites";
 	return "tid: " + tid;
}


function initContentFinding(){

	//default content finding - adding cookie value to all links
	$j('a').attr('data-content-finding', 'Community');

	//Footer Social Media Exit Links
	$j('ul#footer-social a').attr('data-content-finding','Community Footer');
	$j('ul#footer-social a').attr('data-content-subfinding','FOLLOW US');
	
	//Footer News Links
	$j('div.newsItem a').attr('data-content-finding','Community Footer');
	$j('div.newsItem a').attr('data-content-subfinding','NEWS');
	
	$j("div.dtmFooter").each(function(){		
		var title = $j(this).find("h4").first().text();
		title = title == "" ? "Privacy": title;
		$j(this).attr('data-dtmname',title);
	});
	
	//Header Content Finding for social media links in header - No longer in the header, Confirm with Jive Project Team
	// PreCloud Migration - $j('header#j-header div.socialBarGuest a').attr("data-content-finding", "Community Header");
	// PreCloud Migration - $j('header#j-header div.socialBar a').attr("data-content-finding", "Community Header");

	$j("footer#j-footer > ul.dnv > li:last").children("a").each(function(){
		$j(this).attr("data-content-finding", "Community Footer");
	});	

	//Header & Search Content Finding, no updates required for cloud migration

	$j("input#autosearch").keydown(function(evnt){ //No updates Required
		var code = evnt.keyCode || evnt.which;
		if(code == 13){
			setAdobeCookie('contentfinding', 'Community Header', true, 1);
			contentFindingSet = true;
			/*
			if (typeof s_account != 'undefined') {
				var s1=s_gi(s_account);
				s1.ignoreCookieCheck = true;
				s1.c_w('contentfinding', 'Community Header');
				contentFindingSet = true;
			}
			*/
		}
		return;	
	});

	$j(document).click(function(evnt) {
		if(evnt.which == 3)
			return;

		//This event handler is to set content finding

		$this = $j(evnt.target);
		if($this.prop("tagName").toLowerCase() != "a"){
			if($this.parents("a").length > 0){
				$this = $this.parents("a").first();
			}
		}

		if($this.prop("tagName").toLowerCase() == "a"){
			var contentFindingMethod = 'Community';
			var hrf = $this.attr("href");
			if(typeof hrf == 'undefined')
				hrf = "";

			if(hrf == '#')
				hrf = "";

			if(hrf == "")
				return;

			if($this.parents("div.j-search-results-main-container").length > 0 && hrf.indexOf("#facet=") != 0 || $this.parents("div.j-search-results-aside-container").length > 0 && hrf.indexOf("#facet=") != 0){ //No Updates Required
				contentFindingMethod = 'Community Search';
				setContentFinding($this, contentFindingMethod);
				return;
			}

			if($this.parents("li.j-simple-nav-logo,li#jive-nav-link-home,li#jive-nav-link-work,li#jive-nav-link-conx,li#jive-nav-link-places").length > 0){  //if($this.parents("li#jive-nav-link-home,li#jive-nav-link-work,li#jive-nav-link-conx,li#jive-nav-link-places").length > 0){	//No Updates Required	
				contentFindingMethod = 'Community Header';
				setContentFinding($this, contentFindingMethod);
				return;
			}

			/* Pre Cloud Migration, Confirm with Client Team
			if($this.parents("div#fslMainMenu").length > 0){ 
				contentFindingMethod = 'Community Header';
				setContentFinding($this, contentFindingMethod);
				return;
			}
			 */

			if($this.parents("div#appQuickLaunchMenu").length > 0){ // No Updates Required
				contentFindingMethod = 'Community Header';
				setContentFinding($this, contentFindingMethod);
				return;
			}

			if($this.parents("div#menuCreate").length > 0){
				contentFindingMethod = 'Community Header';
				setContentFinding($this, contentFindingMethod);
				return;
			}

			if(($this.parents("div#hd").length > 0 || $this.parents("ul#j-satNav").length > 0) && $this.attr("data-content-finding") != "Community Header"){ //Pre Cloud Migration - if($this.parents("header#j-header").length > 0 && ($this.parents("h1#logo").length > 0 || $this.parents("ul#j-satNav").length > 0))
				contentFindingMethod = 'Community Header';
				setContentFinding($this, contentFindingMethod);
				return;
			}

			if($this.parents("div#j-satNav-menu").length > 0 && $this.attr("data-content-finding") != "Community Header"){ //No Updates Required
				contentFindingMethod = 'Community Header';
				setContentFinding($this, contentFindingMethod);
				return;
			}
			if($this.parents("footer#j-footer").length > 0 && $this.parents("ul.dnv").length > 0 && $this.attr("data-content-finding") != "Community Footer"){				
				contentFindingMethod = 'Community Footer';
				setContentFinding($this, contentFindingMethod);
				return;
			}
			if($this.attr("data-content-finding") != "Community Footer" && $this.attr("data-content-finding") != "Community Header"){
				setContentFinding($this, contentFindingMethod);
			}
		}
	});

	$j(document).mousedown(function(event){
//		if(event.which != 3)
//		return;

		//This event handler is to track downloads

		linkType = "UNKNOWN";

		$this = $j(event.target);
		if($this.prop("tagName").toLowerCase() != "a"){
			if($this.parents("a").length > 0){
				$this = $this.parents("a").first();
			}
		}

		if($this.prop("tagName").toLowerCase() == "a"){
			var linkURL = $this.attr("href");

			if(typeof linkURL == 'undefined')
				linkURL = '';

			if(linkURL == "#")
				linkURL = '';			

			if(linkURL != ''){
				var fullURL = $this.prop("href");
				var downloadType = getDownloadType(linkURL, fullURL);			

				linkType = downloadType;

				if(downloadType != 'Freescale Link' && downloadType != 'Community Link' && downloadType != 'Freescale Link Old' && downloadType != ''){
					trackDwnLoad(linkURL, downloadType, "Community");
				}
			}
		}
	});
}

function setContentFinding(linkObj, contentFindingMethod){

	var linkURL = linkObj.attr("href");

	s_download = 'linkURL = ' + linkURL;

	if(typeof linkURL == 'undefined')
		linkURL = '';

	if(linkURL == "#")
		linkURL = '';

	if(linkURL != ''){
		var fullURL = linkObj.prop("href");

		if(linkType == "UNKNOWN")
			linkType = getDownloadType(linkURL, fullURL);

		if(linkType == 'Freescale Link' || (linkType == 'Community Link' && contentFindingMethod != 'Community')){
			if(!isCrossDomain(fullURL))
				setAdobeCookie('contentfinding', contentFindingMethod, true, 1);
			else{
				if(contentFindingMethod == "Community Header"){
					linkURL += linkURL.indexOf("?") > -1 ? "&": "?";
					linkObj.attr("href", linkURL + "comh=1");
				}if(contentFindingMethod == "Community Footer"){
					linkURL += linkURL.indexOf("?") > -1 ? "&": "?";
					linkObj.attr("href", linkURL + "comf=1");
				}if(contentFindingMethod == "Community Search"){
					linkURL += linkURL.indexOf("?") > -1 ? "&": "?";
					linkObj.attr("href", linkURL + "coms=1");
				}
			}
		}
	}
	linkType = "UNKNOWN";
}


function initSearchTracking(){
	if(searchIn == "Freescale Community"){
		if($j('div#j-main-results > ol.j-search-results > li').length == 1 && $j('div#j-main-results > ol.j-search-results > li.j-loading-bar').length == 1){ // No Updates Required
			//search not complete, retry
			if(setTimeSearch != 0){
				clearTimeout(setTimeSearch);
				setTimeSearch = 0;
			}
			setTimeSearch = setTimeout(initSearchTracking, 250);
			return;
		}	
	}else{
		if($j('div#j-engine-results-1000 > ol.j-search-results > li').length == 1 && $j('div#j-engine-results-1000 > ol.j-search-results > li.j-loading-bar').length == 1){ //No Updates Required
			//search not complete, retry
			if(setTimeSearch != 0){
				clearTimeout(setTimeSearch);
				setTimeSearch = 0;
			}
			setTimeSearch = setTimeout(initSearchTracking, 250);
			return;
		}
	}

	s_searchCount++;

	if($j("div.j-search-results-main-container").length > 0){ //no updates required
		$j("div.j-search-results-main-container").children("a").attr('data-content-finding', 'Community Search');
	}

	$j("#j-search-form").unbind('submit.analytics'); // no updates required
	$j("#j-search-form").bind('submit.analytics', function(){ //no updates required
		if(setTimeSearch != 0){
			clearTimeout(setTimeSearch);
			setTimeSearch = 0;
		}
		setTimeSearch = setTimeout(initSearchTracking, 500); 
	});

	$j(".j-facets a, .j-sub-facets a").unbind('click.analytics'); //No updates Required
	$j(".j-facets a, .j-sub-facets a").bind('click.analytics', function(){ //No updates Required
		if(setTimeSearch != 0){
			clearTimeout(setTimeSearch);
			setTimeSearch = 0;
		}
		setTimeSearch = setTimeout(initSearchTracking, 500); //No Updates Required
	});

	$j(".j-view-more a").unbind('click.analytics');
	$j(".j-view-more a").bind('click.analytics', function(){
		if(setTimeSearch != 0){
			clearTimeout(setTimeSearch);
			setTimeSearch = 0;
		}
		setTimeSearch = setTimeout(initSearchTracking, 500);		
	});

	$j('.jive-autocomplete-search-selected a.remove').unbind('click.analytics');
	$j('.jive-autocomplete-search-selected a.remove').bind('click.analytics',function(){
		if(setTimeSearch != 0){
			clearTimeout(setTimeSearch);
			setTimeSearch = 0;
		}
		setTimeSearch = setTimeout(initSearchTracking, 500);
	});

	$j('input#nameonly[type="checkbox"]').unbind('click.analytics');
	$j('input#nameonly[type="checkbox"]').bind('click.analytics', function() {
		if(setTimeSearch != 0){
			clearTimeout(setTimeSearch);
			setTimeSearch = 0;
		}		
		setTimeSearch = setTimeout(initSearchTracking, 500);
	});	

	$j('a[href|="#facet="]').unbind('click.analytics');
	$j('a[href|="#facet="]').bind('click.analytics', function(){
		if(setTimeSearch != 0){
			clearTimeout(setTimeSearch);
			setTimeSearch = 0;
		}		
		setTimeSearch = setTimeout(initSearchTracking, 500);		
	});	

	$j('select#sort').unbind('change.analytics');
	$j('select#sort').bind('change.analytics', function(){
		if(setTimeSearch != 0){
			clearTimeout(setTimeSearch);
			setTimeSearch = 0;
		}		
		setTimeSearch = setTimeout(initSearchTracking, 500);
	});

	$j('input#author').unbind('focus.analytics');
	$j('input#author').bind('focus.analytics', function(){
		if(setIntAuth != 0){
			clearInterval(setIntAuth);
			setIntAuth = 0;
		}
		setIntAuth = setInterval(waitForAutoComplete, 100);
	});	

	$j('input#place').unbind('focus.analytics');
	$j('input#place').bind('focus.analytics', function(){
		if(setIntPlace != 0){
			clearInterval(setIntPlace);
			setIntPlace = 0;
		}
		setIntPlace = setInterval(waitForAutoComplete, 100);
	});		

	$j('input#author').unbind('focusout.analytics');
	$j('input#author').bind('focusout.analytics', function(){
		if(setIntAuth != 0){
			clearInterval(setIntAuth);
			setIntAuth = 0;
		}
	});	

	$j('input#place').unbind('focusout.analytics');
	$j('input#place').bind('focusout.analytics', function(){
		if(setIntPlace != 0){
			clearInterval(setIntPlace);
			setIntPlace = 0;
		}
	});

	trackSearchForAnalytics();
}

function waitForAutoComplete(){
	$autoComplete = $j('div.j-autocomplete li.user-autocomplete-item').find('a');
	if($autoComplete.length > 0){
		$autoComplete.unbind('click.analytics');
		$autoComplete.bind('click.analytics', function(){
			setTimeout(initSearchTracking, 500);		
		});
	}
}

function trackSearchForAnalytics(){
	var searchResultCount = "non-zero";	

	if(searchIn == "Freescale Community"){
		if($j("#j-main-results .j-no-results").length > 0){	
			searchResultCount = "zero";
		}
	}else{
		if($j("#j-engine-results-1000 .j-no-results").length > 0){	
			searchResultCount = "zero";
		}
	}
	s_searchLog = searchResultCount;

	s_log = 'get s object for search';
	digitalData.pageInfo.pageLoadRule="CommunityGeneral";
	digitalData.pageInfo.pageURLClean = getPageURLForAnalytics();

	digitalData.pageInfo.pageName = truncateString("Community: "+title, pageNameLen);
	digitalData.pageInfo.pageType = "Community Search";
	digitalData.pageInfo.siteSection1 = "Community";					
	digitalData.pageInfo.siteSection2 = "Community Search";
	digitalData.pageInfo.pageEventPathing = "D=pageName";
	digitalData.pageInfo.contentFinding  = getContentFinding(document.referrer, document.URL);
	digitalData.pageInfo.assetID = "D=pageName";
	digitalData.siteInfo.sitePlatform = "full";
	digitalData.siteInfo.lang = "en";
	digitalData.pageInfo.localTitle = "D=pageName";
	digitalData.communityInfo.parentCommunity = parentSpace;
	digitalData.communityInfo.tab = tab;
	digitalData.communityInfo.contentID = contentId;
	digitalData.communityInfo.category = category;
	digitalData.pageInfo.tags = tags;
	digitalData.communityInfo.activityType = activityType;

	searchTerm = $j('#j-search-input').val();
	digitalData.searchInfo.searchkeyword = searchTerm;
	digitalData.searchInfo.resultCount = searchResultCount;
	digitalData.searchInfo.searchFilterList = getFilterFromQueryString();

	digitalData.userInfo.userName = username;
	_satellite.pageBottom();
	digitalData.pageInfo.contentFinding  = "";
	s_log = 'page tracked';
}

/*
function trackSearchForAnalytics(){
	var searchResultCount = "non-zero";	

	if(searchIn == "Freescale Community"){
		if($j("#j-main-results .j-no-results").length > 0){	
			searchResultCount = "zero";
		}
	}else{
		if($j("#j-engine-results-1000 .j-no-results").length > 0){	
			searchResultCount = "zero";
		}
	}
	s_searchLog = searchResultCount;

	if(s){
		s_log = 'get s object for search';
		s.manageVars("clearVars","events",1);
		s.pageURL = getPageURLForAnalytics();
		s.pageName = truncateString("Community: "+title, pageNameLen);
		s.prop1 = "Community Search";
		s.prop2 = "Community Search";					
		s.channel = "Community";					
		s.prop16 = "D=pageName";
		s.eVar1  = getContentFinding(document.referrer, document.URL);
		s.eVar16 = "D=pageName";
		s.eVar17 = "full";
		s.eVar18 = "en";
		s.eVar19 = "D=pageName";
		s.eVar23 = getQryParamForAnalytics(document.URL, "tid");
		s.eVar25 = parentSpace;
		s.eVar26 = tab;
		s.eVar27 = contentId;
		s.eVar28 = category;
		s.eVar29 = tags;
		s.eVar30 = activityType;

		searchTerm = $j('#j-search-input').val();
		s.eVar2 = searchTerm;
		s.prop4 = searchTerm;
		s.prop5 = searchResultCount;
		s.eVar31 = searchTerm;
		s.list1 = getFilterFromQueryString();

		s.eVar32 = username;
		s.eVar33 = userid;
		s.t();
		s.eVar1 = '';
		s.events = '';
		s_log = 'page tracked';
	}	
}
*/
function getFilterFromQueryString(){	
	var filterString = '';

	var defaultFacet = true;
	var excludeParams = ';q;sort;';

	if(searchIn == "Freescale Community"){
		var urlParts = document.URL.split('?');

		if(urlParts.length > 1){
			var queryString = urlParts[1];
			var queryParams = queryString.split('&');
			for(i=0;i<queryParams.length;i++){
				var queryParam = queryParams[i].split('=');
				if(queryParam.length > 1){
					var paramName = queryParam[0];
					var paramValue = decodeURIComponent(queryParam[1]);
					if(excludeParams.indexOf(';' + paramName + ';') == -1 && paramValue != ''){
						filterString += '$' + paramName + '~' + paramValue;
						if(paramName == 'facet'){
							defaultFacet = false;
						}
					}
				}
			}
		}		 
	}else{
		defaultFacet = false;
	}	

	if(defaultFacet){
		filterString = "$facet~content" + filterString;
	}
	filterString = 'searchIn~' + searchIn + filterString;

	return filterString;
}

function getQryParamForAnalytics(url,name)
{
	var regexS = "[\\?&]"+name+"=([^&#]*)";
	var regex = new RegExp( regexS );
	var tmpURL = url;
	var results = regex.exec( tmpURL );
	if( results == null )
		return "";
	else
		return results[1];
}

function getDownloadType(linkURL, fullURL){
	var j;

	//List of Freescale Domains
	//var fslDomains = ['//uat.freescale.com', '//cache-uat.freescale.com', '//www.freescale.com', '//cache.freescale.com', '//freescale.com'];
	var fslDomains = ['//uat.freescale.com', '//cache-uat.freescale.com', '//www.nxp.com', '//cache.nxp.com', '//nxp.com'];

	//List of Old Ch & Jp domains
	var fslExDomains = ['//freescale.co.jp', '//www.freescale.co.jp', '//www.freescale.com.cn', '//freescale.com.cn'];

	//List of FSL Community Domains
	var commDomains = ['//community-uat.freescale.com', '//community.freescale.com', '//freescale.jiveon.com'];  // Pre Cloud Migration - var commDomains = ['//community-uat.freescale.com', '//community.freescale.com'];

	//List of teamsite abstracts that should not be treated as downloads
	var nonsechtmlExcludeList = ['/files/abstract/', '/files/online_tools/eTPU/', 'elqNow/elqBlank.htm', '/ruhp/myfreescale.html', '/ruhp_weblogic/myfreescale.html', '/files/graphic/block_diagram'];

	//List of teamsite abstracts or file extensions that should be treated as downloads
	var nonsecfslIncludeList = ['/files/', '/lgfiles/', '/mcds/', '/zh-Hans/files/', '/ja/files/', '.pdf', '.doc', '.zip', '.ppt', '.txt', '.iso', '.tar'];

	//List of file extensions that should be treated as downloads
	var nonsecIncludeList = ['.pdf', '.doc', '.zip', '.ppt', '.txt', '.iso', '.tar'];

	var downloadType = '';

	linkURL = linkURL.toLowerCase();
	var linkURLPart = linkURL.split('?')[0];
	fullURL = fullURL.toLowerCase();
	fullURL = fullURL.split('?')[0];

	//Check: URL is a freescale URL
	var isFSLUrl = false;
	var isExFSLUrl = false;
	for (j = 0; j < fslDomains.length; j++) {
		isFSLUrl = isFSLUrl || (linkURLPart.indexOf(fslDomains[j]) != -1);
	}

	for (j = 0; j < fslExDomains.length; j++) {
		isExFSLUrl = isExFSLUrl || (linkURLPart.indexOf(fslExDomains[j]) != -1);
	}

	isFSLUrl = isFSLUrl && !isExFSLUrl;

	if(isFSLUrl){
		//In: URL is a freescale URL

		if(downloadType == '' && linkURLPart.indexOf('/webapp/download') > -1){
			downloadType = 'Registered';
		}
		if(downloadType == '' && linkURLPart.indexOf('/download/mod_download.jsp') > -1){
			downloadType = 'Moderated';
		}
		if(downloadType == '' && linkURLPart.indexOf('/download/license.jsp') > -1){
			downloadType = 'Licensed';
		}
		if(downloadType == '' && linkURLPart.indexOf('/download/files/') > -1){
			downloadType = 'Marketing Leveraged';
		}

		if(downloadType == '' && linkURLPart.indexOf(".htm")!=-1){
			//In: URL is pointing to a html

			//Check: Exclude tracking for htmls that are already tracked in page view
			var doExclude = false;
			for (j = 0; j < nonsechtmlExcludeList.length; j++) {
				doExclude = doExclude || (linkURLPart.indexOf(nonsechtmlExcludeList[j]) != -1);
			}

			if(!doExclude)
				downloadType = 'Non-Secured';			
		}else if(downloadType == ''){
			//In: URL is not pointing to a html

			var doInclude = false;
			for (j = 0; j < nonsecfslIncludeList.length; j++) {
				doInclude = doInclude || (linkURLPart.indexOf(nonsecfslIncludeList[j]) != -1);
			}		

			if(doInclude)
				downloadType = 'Non-Secured';
		}

		if(downloadType == '')
			downloadType = 'Freescale Link';
	}else{
		//In: URL is not a freescale URL

		//find if href URL is community URL
		var isCommunityURL = false;

		for(j=0;j<commDomains.length;j++){
			isCommunityURL = isCommunityURL || (fullURL.indexOf(commDomains[j]) != -1);
		}

		var doInclude = false;
		for (j = 0; j < nonsecIncludeList.length; j++) {
			doInclude = doInclude || (linkURLPart.indexOf(nonsecIncludeList[j]) != -1);
		}				

		if(isCommunityURL && (linkURL.indexOf('/servlet/jiveservlet/download') > -1 || doInclude)){
			downloadType = 'Non-Secured';
		}else if(isCommunityURL){
			downloadType = 'Community Link';
		}else if(isExFSLUrl && doInclude){
			downloadType = 'Non-Secured';
		}else if(isExFSLUrl){
			downloadType = 'Freescale Link Old';
		}
	}

	return downloadType;
}

function trackDwnLoad(linkURL, downloadType, contentFindingMethod){
	s_download = 'trackSecuredDwnLoad = ' + downloadType;

	activityType = 'download';

	if(linkURL.indexOf('/') == 0){
		linkURL = document.domain + linkURL;
	}

	if (typeof s_account != 'undefined') {
		var s1=s_gi(s_account);

		s1.linkTrackVars='channel,prop1,prop2,prop3,prop6,prop8,prop9,prop10,' +
		'prop11,prop13,prop14,prop16,prop18,prop21,prop74,eVar1,eVar2,eVar3,' +
		'eVar4,eVar5,eVar6,eVar15,eVar16,eVar17,eVar18,eVar19,eVar24,' +
		'eVar25,eVar26,eVar27,eVar28,eVar29,eVar30,eVar31,eVar32,eVar33,' +
		'eVar34,eVar35,eVar38,eVar39,eVar40,eVar48,eVar51,eVar52,eVar53,events';

		var hrefUrl = linkURL.split("?")[0];
		hrefUrl = hrefUrl.split("#")[0];
		hrefUrl = hrefUrl.replace("http://","");
		hrefUrl = hrefUrl.replace("https://","");
		if(downloadType != 'Non-Secured' && downloadType != 'Marketing Leveraged'){
			hrefUrl = hrefUrl + '?colCode='+ getQryParamForAnalytics(linkURL,'colCode');
		}

		hrefUrl = hrefUrl.replace('cache.freescale.com', 'www.freescale.com');		

		if(downloadType != 'Non-Secured'){
			s1.linkTrackEvents='event15';
			s1.events = 'event15';
			s1.prop16 = 'dwnld start:'+hrefUrl;  // Event Pathing
		}else{
			s1.linkTrackEvents='event15,event11';
			s1.events = 'event15,event11';
			s1.prop16 = 'dwnld:'+hrefUrl;  // Event Pathing
		}
		s.eVar30 = activityType;
		s1.eVar40=downloadType;
		s1.eVar16= linkURL;
		s1.eVar1 = contentFindingMethod;

		if(downloadType != 'Non Secured' && downloadType != 'Moderated'){
			setAdobeCookie('doc_nodeid',s1.eVar20);
			setAdobeCookie('parent_sc_channel',s1.channel);
			setAdobeCookie('parent_sc_prop1',s1.prop1);
			setAdobeCookie('parent_sc_prop2',s1.prop2);
			setAdobeCookie('parent_sc_prop3',s1.prop3);
			setAdobeCookie('parent_sc_prop6',s1.prop6);
			setAdobeCookie('parent_sc_prop8',s1.prop8);
			setAdobeCookie('parent_sc_prop9',s1.prop9);
			setAdobeCookie('parent_sc_prop10',s1.prop10);
			setAdobeCookie('parent_sc_prop11',s1.prop11);
			setAdobeCookie('parent_sc_prop21',s1.prop21);
			setAdobeCookie('parent_sc_eVar1',s1.eVar1);
			setAdobeCookie('parent_sc_eVar2',s1.eVar2);
			setAdobeCookie('parent_sc_eVar3',s1.eVar3);
			setAdobeCookie('parent_sc_eVar19',s1.eVar19);
			setAdobeCookie('parent_sc_eVar21',s1.eVar21);
			setAdobeCookie('parent_sc_eVar34',s1.eVar34);
			setAdobeCookie('parent_sc_eVar35',s1.eVar35);
			setAdobeCookie('parent_sc_eVar48',s1.eVar48);
			setAdobeCookie('parent_sc_pageName',s1.pageName);
			setAdobeCookie('parent_sc_commact',s1.eVar30);
		}

		s1.eVar24=hrefUrl;
		s1.tl(this,'d','Download:'+hrefUrl);
		s1.eVar1 = "";
		s1.events = '';
	}
}

function truncateString(str,len){
	var truncatedStr = str;
	if(str.length > len){
		truncatedStr = str.substring(0,(len-3)) + "...";
	}
	return truncatedStr;
}

function isDTMEnabled(){
	var dtmEnabled = false;
	if(typeof _satellite != 'undefined'){
		dtmEnabled = true;
	}
	return dtmEnabled;
}

function setAdobeCookie(name, value, ignoreCookieCheck, days) {
	if(typeof days == "undefined") 
		days = 1;
    var d = new Date();
    d.setTime(d.getTime() + (days*24*60*60*1000));
    var expires = "expires=" + d.toGMTString();
    name = "_sc_" + name;
    document.cookie = name + "=" + value + "; expires=" + expires + "; domain=" + getMajorDomain() + "; path=/";
}

function getAdobeCookie(cname) {
    var name = "_sc_" + cname + "=";
    var ca = document.cookie.split(';');
    for(var i=0; i<ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1);
        if (c.indexOf(name) == 0) {
            return c.substring(name.length, c.length);
        }
    }
    return "";
}

function removeAdobeCookie(name){
	var value = "";
    var expires = "Thu, 01 Jan 1970 00:00:00 UTC";
    name = "_sc_" + name;
    document.cookie = name + "=" + value + "; expires=" + expires + "; domain=" + getMajorDomain() + "; path=/";
}

function getMajorDomain(){
	return document.domain.substring(document.domain.indexOf("."));
}

function isCrossDomain(s_hrefUrl){	
	var s_domain = document.domain;
	s_domain = s_domain.replace(/^[^.]+\./g, "");	
	var s_hrefUrl_arr = s_hrefUrl.replace('http://','').replace('https://','').replace(/^[^.]+\./g, "").split(/[/?#]/);
		
	if(s_domain == s_hrefUrl_arr[0]) {
		return false;
	}
	return true;  
}