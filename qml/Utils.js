.pragma library
.import harbour.matkakortti 1.0 as MatkaKortti

function moneyString(value) {
    return value ? ((value/100.0).toFixed(2) + " €") : ""
}

function dateString(date) {
    return date.toLocaleDateString(Qt.locale(), "dd.MM.yyyy" + MatkaKortti.HarbourSystemTime.notification)
}

function timeString(date) {
    return date.toLocaleTimeString(Qt.locale(), "hh:mm" + MatkaKortti.HarbourSystemTime.notification)
}

function dateTimeString(date) {
    return dateString(date) + " " + timeString(date)
}
