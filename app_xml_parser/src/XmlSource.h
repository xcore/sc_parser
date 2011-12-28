#ifndef XML_SOURCE_H_INCLUDED
#define XML_SOURCE_H_INCLUDED

/******** data to be parsed *********/
static unsigned char Source[] = {"\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<!-- sample xml -->\
<breakfast_menu>\
	<food0>\
		<name>Belgian Waffles</name>\
		<price>$5.95</price>\
		<description>two of our famous Belgian Waffles with maple syrup</description>\
		<calories>650</calories>\
	</food0>\
	<food1>\
		<name>Strawberry Belgian Waffles</name>\
		<price>$7.95</price>\
		<description>light Belgian waffles covered with strawberries</description>\
		<calories>900</calories>\
	</food1>\
	<food2>\
		<name>Berry-Berry Belgian Waffles</name>\
		<price>$8.95</price>\
		<description>light Belgian waffles covered with an assortment of berries</description>\
		<calories>900</calories>\
	</food2>\
	<food3>\
		<name>French Toast</name>\
		<price>$4.50</price>\
		<description>thick slices made from our homemade sourdough bread</description>\
		<calories>600</calories>\
	</food3>\
	<food4>\
		<name>Homestyle Breakfast</name>\
		<price>$6.95</price>\
		<description>two eggs, bacon or sausage, toast</description>\
		<calories>950</calories>\
	</food4>\
</breakfast_menu>\
"};

#endif
