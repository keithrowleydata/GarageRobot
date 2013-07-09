<%@ Page Title="Home Page" Language="VB" MasterPageFile="~/Site.Master" AutoEventWireup="false"
    CodeFile="Default.aspx.vb" Inherits="_Default" %>

<asp:Content ID="HeaderContent" runat="server" ContentPlaceHolderID="HeadContent">
</asp:Content>
<asp:Content ID="BodyContent" runat="server" ContentPlaceHolderID="MainContent">
<div class="centercont">
    <h2>
        <asp:Label ID="txt1" runat="server" Text=""></asp:Label>
    </h2>
    <br />
    <p>
        <asp:Button ID="btnSerialData" runat="server" Text="Refresh" Width="200px" Height="50px" />
    </p>
    <br />
    <p>
        <asp:Button ID="btnClickGarage" runat="server" Text="Click Garage Door" Width="200px" Height="50px" />
    </p>
</div>
</asp:Content>