var map = L.map('map');
map.setView([51.505, -0.09], 13);

L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

// ================================= // ================================= //
// drawning
// ================================= // ================================= //
var marker = L.marker([51.5, -0.09]).addTo(map);
map.on("click", onMapClick);

var popup = L.popup();
function onMapClick(e) {
        popup
                .setLatLng(e.latlng)
                .setContent("You clicked the map at " + e.latlng.toString())
                .onMap();
}


// ================================= // ================================= //
// browser tracking
// ================================= // ================================= //
// navigator.geolocation.watchPosition(watchPositionSuccess, watchPositionError);

// let marker, circle, zoomed = false;
// function watchPositionSuccess(pos) {
//         const lat = pos.coords.latitude;
//         const lng = pos.coords.longitude;
//         const accuracy = pos.coords.accuracy;
//
//         if (marker) {
//                 map.removeLayer(marker);
//                 map.removeLayer(circle);
//         }
//
//         marker = L.marker([lat, lng]).addTo(map);
//         circle = L.circle([lat, lng], {radius: accuracy}).addTo(map);
//         console.log(`lat=${lat},lng=${lng}`);
//
//         if (!zoomed) {
//                 map.fitBounds(circle.getBounds());
//                 zoomed = true;
//         }
//
//         map.setView([lat, lng]);
// }
// function watchPositionError(err) {
//         console.log(err);
//         switch (err.code) {
//                 case 1:
//                         alert("Permission denied");
//                         break;
//
//                 case 2:
//                         // alert("Position unavailable");
//                         break;
//
//                 case 3:
//                         alert("Timeout");
//                         break;
//
//                 default:
//                         alert("Unknown error");
//         }
// }

