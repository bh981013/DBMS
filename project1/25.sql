select name
from Pokemon
where id in
(
select distinct pid
from CatchedPokemon
where pid in
        (select c.pid
        from CatchedPokemon c, Trainer t
        where t.id = c.owner_id and t.hometown = 'Sangnok City')
      and pid in
        (select c.pid
        from CatchedPokemon c,Trainer t
        where t.id = c.owner_id and t.hometown = 'Brown City')
)
order by name;