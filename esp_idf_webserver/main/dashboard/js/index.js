var pos = [40.179479762376175, -8.418097439674227];
var map = L.map('map');
map.setView(pos, 13);

L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
        maxZoom: 19,
        attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);

var trajectory = [pos];
var marker = L.marker(pos).addTo(map);
var polygon = L.polygon([pos]).addTo(map);
var popup = L.popup()

function onMapClick(e) {
        popup
                .setLatLng(e.latlng)
                .setContent("You clicked the map at " + e.latlng.toString())
                .openOn(map);
        trajectory.push(e.latlng);

        polygon.addLatLng(e.latlng);
        console.log(trajectory);
}
map.on("click", onMapClick);

function clearTrajectory() {
        trajectory = [pos];
        polygon.setLatLngs([pos])
}

document.querySelector("#clearTrajectoryButton").addEventListener("click", () => {
        clearTrajectory();
        console.log("trajectory cleared");
})
document.querySelector("#sendTrajectoryButton").addEventListener("click", () => {
        fetch("/trajectory", {
                method: "POST",
                body: JSON.stringify(trajectory)
        }).then((res) => {
                if (!res.ok) {
                        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
                }

                console.log("trajectory sent successfully");
                clearTrajectory();

        }).catch((err) => {
                console.log(`failed to send trajectory: ${err.message}`);
        })
})

