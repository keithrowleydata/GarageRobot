Partial Class _Default
    Inherits System.Web.UI.Page

    Protected Sub Page_Load(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Load
        If Not Page.IsCallback Then
            Dim GD As New GarageDoor
            txt1.Text = GD.GetStatus()
        End If
    End Sub

    Protected Sub btnSerialData_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnSerialData.Click
        Dim GD As New GarageDoor
        txt1.Text = GD.GetStatus()
    End Sub

    Protected Sub btnClickGarage_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles btnClickGarage.Click
        Dim GD As New GarageDoor
        GD.ClickGarageDoor()
        txt1.Text = "Garage Door Clicked"
    End Sub
End Class
